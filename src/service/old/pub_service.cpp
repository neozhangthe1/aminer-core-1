#include <limits>
#include <sstream>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

#include "pub_pervice.h"
#include "search.h"
#include "index_resource.h"
#include "SearchConfigure.h"
#include "PublicationAdapter.h"
#include "Timer.h"
#include "AuthorAdapter.h"
#include "interface.pb.h"
#include "InterfaceUtils.h"
#include "google/protobuf/message.h"
#include "Enumerator.h"
#include "SearchFilter.h"
#include "AdvancedSearch.h"
#include "ProcessQuery.h"
#include "Logger.h"
#include "PublicationSearcher.h"
#include "PublicationUtils.h"
#include "AuthorNameDisambiguation.h"
#include "BackstageTask.h"
#include "stringutils.h"
#include "PaperRecommend.h"

using namespace boost;
using namespace boost::algorithm;

namespace PubService{

	//To Kitty: Please rewirite this function using the rerank approach
	/*
	* Input: mashaler::StringQueryParams
	* Output: mashaler::PublicationResult
	*/
	bool searchPublications(const string& input, string& output)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		string query = params.query();
		LOG(INFO) << boost::str(boost::format("[search]%s")%query);
		string orderby;
		ProcessQuery::get_param(query, "orderby", orderby);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();

		IndexList* index = resource.pubIndex();
		PublicationSearcher searcher(index);
		QueryItemEnumerator* enumerator = searcher.search(query);

		mashaler::PublicationResult result;

		if(enumerator)
		{
			QueryItem item;
			if(!orderby.empty())
			{
				if(orderby=="year")
				{
					OrderedQueryItemEnumerator* new_enumer = new OrderedQueryItemEnumerator();
					while(enumerator->next(item))
					{
						new_enumer->addQueryItem(item);
					}
					PubYearComparer* comp = new PubYearComparer();
					new_enumer->setComparer(comp);
					delete enumerator;
					enumerator = new_enumer;
				}
				else if(orderby=="citation")
				{
					OrderedQueryItemEnumerator* new_enumer = new OrderedQueryItemEnumerator();
					while(enumerator->next(item))
					{
						new_enumer->addQueryItem(item);
					}
					PubCiteComparer* comp = new PubCiteComparer();
					new_enumer->setComparer(comp);
					delete enumerator;
					enumerator = new_enumer;
				}
			}

			result.set_total_count(enumerator->size());

			for(int i=0;i < params.offset() && enumerator->next(item); ++i);

			Document* doc;
			for(int i=0;i < params.count() && enumerator->next(item); ++i)
			{
				doc = publicationCollection->getDocumentByIndex(item.docId);
				assert(doc!=NULL);
				ProtoConverter::convertPub(publicationCollection, *doc, *result.add_publications(), params.returned_fields());
			}
			delete enumerator;
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	* Input: mashaler::StringQueryParams
	* Output: mashaler::PublicationScoredResult
	*/
	bool searchPublicationsWithScore(const string& input, string& output)//(string& query, int offset, int count, vector<string>& searchedFields, vector<string>& returnedFields)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		/*vector<string> searchedFields;
		for(int i=0;i<params.searched_fields().size();++i)
		searchedFields.push_back(params.searched_fields(i));*/
		LOG(INFO) << boost::str(boost::format("[search]%s")%params.query());
		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();

		IndexList* index = resource.pubIndex();
		Searcher searcher(index, new StandardQueryAnalyzer(index));
		searcher.SetSearchedFields(SearchConfigure::instance().pubDefSearchFields);
		QueryItemEnumerator* enumerator = searcher.Search(params.query());

		mashaler::PublicationScoredResult result;

		if(enumerator)
		{
			result.set_total_count(enumerator->size());
			QueryItem item;
			mashaler::PubScorePair* pair = NULL;

			for(int i=0;i < params.offset() && enumerator->next(item); ++i);

			Document* doc;
			for(int i=0;i < params.count() && enumerator->next(item); ++i)
			{
				doc = publicationCollection->getDocumentByIndex(item.docId);
				assert(doc!=NULL);
				pair = result.add_pub_score_pairs();
				pair->set_score(item.score);
				assert(pair->mutable_publication() != NULL);
				ProtoConverter::convertPub(publicationCollection, *doc, *(pair->mutable_publication()), params.returned_fields());
			}
			delete enumerator;
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	* Get pub list by one author'id...(get one by one, skip/not return it if not found)
	*  int offset--from where to start; int count--total pubs to return
	* Author: kitty
	* Time:  2010/06/14
	*  
	* Update by Hang Su <su_hang@live.com>
	* Date: June 17, 2011
	*/
	/*
	* Input: mashaler::StringQueryParams
	* Output: mashaler::PublicationResult
	*/
	bool getPublicationsByNaId(const string& input, string& output)//(int naid, int offset, int count, vector<string>& returnedFields, vector<Condition>& conditions, string sortType)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authors = resource.authorCollection();
		DocumentCollection* publications = resource.pubCollection();
		/*
		AuthorAdapter adapter;
		DocumentCollection* authors = adapter.getDocCollection();
		*/
		assert(params.ids_size()!=0);
		if(params.ids_size()==0)
			return false;

		mashaler::PublicationResult result;
		Author* author;
		author = (Author*)authors->getDocumentByIndex(params.ids(0));

		LOG(INFO) << boost::str(boost::format("[naid]%d")%params.ids(0));

		Document* doc;

		if(author) //author !=NULL
		{
			QueryItemEnumerator* enumerator = new PublicationEnumerator(author->pub_ids);
			if(params.conditions_size())
			{
				Condition con;
				enumerator = new SearchFilter(enumerator, resource.pubCollection());
				for(int i=0;i<params.conditions_size();++i)
				{
					ProtoConverter::convert(con, params.conditions(i));
					((SearchFilter*)enumerator)->addCondition(con);
				}
			}
			QueryItem item;
			vector<Publication*> pubs;
			Document* doc;
			while(enumerator->next(item))
			{
				doc = publications->getDocumentByIndex(item.docId);
				if(doc)
				{
					pubs.push_back((Publication*)doc);
				}
			}
			sort(pubs.begin(), pubs.end(), PubUtils::compare_pub_year);
			for(int i=params.offset();i < params.offset()+params.count() && i<pubs.size(); ++i)
			{
				doc = pubs[i];
				if(doc)
				{
					ProtoConverter::convertPub(resource.pubCollection(), *doc, *result.add_publications(), params.returned_fields());
				}
			}
			result.set_total_count(pubs.size());
			delete enumerator;
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	* Get pub list by one conf'id...(get one by one, skip/not return it if not found)
	*  int offset--from where to start; int count--total pubs to return
	* Author: kitty
	* Time:  2010/06/14
	*
	* Update by Hang Su <su_hang@live.com>
	* Date: June 17, 2011
	*
	* Input: mashaler::IntQueryParams
	* Output: mashaler::PublicationResult
	*/
	bool getPublicationsByConfId(const string& input, string& output)//(int confId, int offset, int count, vector<string>& returnedFields, vector<Condition>& conditions, string sortType)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);
		/*
		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();
		*/
		if(params.ids_size()==0)
			return false;

		mashaler::PublicationResult result;
		Document* doc;
		
		LOG(INFO) << boost::str(boost::format("[conf]%d") % params.ids(0));

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publications = resource.pubCollection();
		JConfMap* jConfMapInstance = JConfMap::getInstance();
		JournalConferenceInfo* journalConferenceInfo;
		journalConferenceInfo =jConfMapInstance->getJConf(params.ids(0));

		if(journalConferenceInfo)
		{
			vector<Publication*> pub_ptrs;
			QueryItemEnumerator* enumerator = new PublicationEnumerator(journalConferenceInfo->pub_ids);
			if(params.conditions_size())
			{
				Condition con;
				enumerator = new SearchFilter(enumerator, resource.pubCollection());
				for(int i=0;i<params.conditions_size();++i)
				{
					ProtoConverter::convert(con, params.conditions(i));
					((SearchFilter*)enumerator)->addCondition(con);
				}
			}
			QueryItem item;
			for(int i=0;i < params.count() && enumerator->next(item); ++i)
			{
				doc = publications->getDocumentByIndex(item.docId);
				if(doc)
				{
					pub_ptrs.push_back((Publication*)doc);
				}
			}
			delete enumerator;
			result.set_total_count(pub_ptrs.size());
			sort(pub_ptrs.begin(), pub_ptrs.end(), PubUtils::compare_pub_citation); 
			for(int i = params.offset(); i< params.offset() + params.count() && i< pub_ptrs.size(); i++)
			{
				doc = pub_ptrs[i];
				ProtoConverter::convertPub(resource.pubCollection(), *doc, *result.add_publications(), params.returned_fields());
			}
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	* Get pub list by pub'id...(get one by one, skip/not return it if not found)
	* Author: kitty
	* Time:  2010/06/11
	*
	* Update by Hang Su <su_hang@live.com>
	* Date: June 17, 2011
	*
	*
	* Input: mashaler::IntQueryParams
	* Output: mashaler::PublicationResult
	*/
	bool getPublicationsById(const string& input, string& output)//(vector<int>& pubId, vector<string>& returnedFields)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();
				
		vector<int> ids;
		for(int i=0; i < params.ids_size(); i++)
		{
			int id = params.ids(i);
			ids.push_back(id);
		}
		LOG(INFO) << boost::str(boost::format("[id]%s") % strutils::vectorToString(ids));
		
		mashaler::PublicationResult result;
		Document* doc;
		result.set_total_count(publicationCollection->getDocCounts());
		for(int i=0; i < params.ids_size(); i++)
		{
			int onePubID = params.ids(i);

			doc = publicationCollection->getDocumentByIndex(onePubID);
			if(doc)
			{
				ProtoConverter::convertPub(resource.pubCollection(), *doc, *result.add_publications(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
	}

	/*
	* get the list of publications cited by the given publication
	* Author: Hang Su <su_hang@live.com>
	* Date: July 5, 2011
	*
	*
	* Input: mashaler::IntQueryParams
	* Output: mashaler::PublicationResult
	*/
	bool getCitePublications(const string& input, string& output)
	{
		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();

		mashaler::PublicationResult result;
		Publication* pub, *cite_pub;
		int offset, count;

		mashaler::IntQueryParams params;
		if(!params.ParseFromString(input))
			goto error;
		if(!params.ids_size())
			goto error;
		if(!params.has_offset() || !params.has_count())
			goto error;
		
		LOG(INFO) << boost::str(boost::format("[cite]%d offset:%d count%d") % params.ids(0) % params.offset() % params.count());

		pub = (Publication*)publicationCollection->getDocumentByIndex(params.ids(0));
		if(!pub) 
			goto error;
		result.set_total_count(pub->cite_pubs.size());
		for(int i=params.offset(); i<params.count() && i<pub->cite_pubs.size(); ++i)
		{
			cite_pub = (Publication*)publicationCollection->getDocumentByIndex(pub->cite_pubs[i]);
			if(cite_pub)
			{
				ProtoConverter::convertPub(resource.pubCollection(), *cite_pub, *result.add_publications(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
error:
		return false;
	}
	/*
	* get the list of publications which cite this publication
	* Author: Hang Su <su_hang@live.com>
	* Date: July 5, 2011
	*
	*
	* Input: mashaler::IntQueryParams
	* Output: mashaler::PublicationResult
	*/
	bool getCitedByPulicataions(const string& input, string& output)
	{
		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();

		mashaler::PublicationResult result;
		Publication* pub, *cited_by_pub;

		mashaler::IntQueryParams params;
		if(!params.ParseFromString(input))
			goto error;
		if(!params.ids_size())
			goto error;
		if(!params.has_offset() || !params.has_count())
			goto error;
		
		LOG(INFO) << boost::str(boost::format("[cited]%d offset:%d count%d") % params.ids(0) % params.offset() % params.count());

		pub = (Publication*)publicationCollection->getDocumentByIndex(params.ids(0));
		if(!pub) 
			goto error;
		result.set_total_count(pub->cited_by_pubs.size());
		for(int i=params.offset(); i<params.count() && i<pub->cited_by_pubs.size(); ++i)
		{
			cited_by_pub = (Publication*)publicationCollection->getDocumentByIndex(pub->cited_by_pubs[i]);
			if(cited_by_pub)
			{
				ProtoConverter::convertPub(resource.pubCollection(), *cited_by_pub, *result.add_publications(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
error:
		return false;
	}

	/*
	* Add a new publication. Authors' IDs given are ignored, but are instead inferred from the authors' names.
	* Date: Oct 17, 2011
	*
	*
	* Input: mashaler::Publication
	* Output: int literal, which is the ID of the added publication. If time-out, -1 is returned.
	* Authors with missing IDs will be added as new authors. (No inference!)
	*/
	bool addPublication( const string& input, string& output )
	{
		mashaler::Publication m_publication;
		m_publication.ParseFromString(input);

		Publication* publication = new Publication(IndexResource::instance().pubCollection());
		ProtoConverter::convertPub(m_publication, *publication);
		
		LOG(INFO) << boost::str(boost::format("[add]%s") % publication->title);
		
		// Add new authors
		{
			vector<string> author_names;
            split(author_names, publication->authors, boost::lambda::_1 == ',');
			publication->authorIDs.resize(author_names.size(), -1);

			for(int i = 0; i!=author_names.size(); ++i)
				if(publication->authorIDs[i] == -1)
				{
					Author* author_ptr = new Author(IndexResource::instance().authorCollection());
					author_ptr->names.push_back(author_names[i]);
					publication->authorIDs[i] = AuthorAdapter::instance().add(author_ptr);
				}
		}

		shared_mutex mx;
		mx.lock();

		BackstageTasks::instance().addTask(new StatelessTask([&](){
			PublicationAdapter::instance().add(publication);

			vector<int> indices;
			indices.push_back(publication->id);
			IndexResource::instance().pubIndex()->addDocumentByIndex(indices);

			mx.unlock();
		}));

		mx.timed_lock(posix_time::minutes(2));

		output = lexical_cast<string>(publication->id);
		return true;
	}

	namespace{
		const Publication* GetPublicationById(int pub_id)
		{
			try
			{
				return dynamic_cast<Publication*>(
					PublicationAdapter::instance().getDocCollection()
					->getDocumentByIndex(pub_id));
			}
			catch(...)
			{
				return NULL;
			}
		}

		void UpdatePublication(Publication* publication)
		{
			PublicationAdapter::instance().getDocCollection()->assign(publication->id, publication, false);
		}

		const Author* GetAuthorById(int author_id)
		{
			try
			{
				return dynamic_cast<Author*>(
					AuthorAdapter::instance().getDocCollection()
					->getDocumentByIndex(author_id));
			}
			catch(...)
			{
				return NULL;
			}
		}

		void UpdateAuthor(Author* author)
		{
			AuthorAdapter::instance().getDocCollection()->assign(author->naid, author, false);
		}
	}

	/*
	* Check whether the names of the publication's authors' matches the IDs. If not, try to fix them automatically.
	* Date: Sep 26, 2011
	*
	* Input: A integer literal, which is the ID of the publication to be revised.
	* Output: None.
	*/
	bool reviseAuthorIdNameMatches(const string& input, string& output)
	{
		int id;
		try{
			id = lexical_cast<int>(input);
		}
		catch(boost::bad_lexical_cast& e)
		{
			auto message = boost::format("Unable to parse ID from string \"%1%\". Exception: %2%") % input % e.what();
			LOG(ERROR) << message.str();
			return false;
		}
		
		LOG(INFO) << boost::str(boost::format("[revise]%d") % id);

		auto old_pub = GetPublicationById(lexical_cast<int>(input));
		if(!old_pub)
		{
			auto message = boost::format("Unable to find publication given ID=\"%1%\"") % id;
			LOG(ERROR) << message.str();
			return false;
		}

		auto publication = new Publication(*old_pub);		

		vector<string> author_names;
		split(author_names, publication->authors, boost::lambda::_1 == ',');

		vector<int>& author_ids = publication->authorIDs; // NOTE this is just a reference, a short hand
		author_ids.resize(author_names.size(), -1);

		vector<Author2pub> toUpdate;
		for(int i = 0; i!=author_names.size(); ++i)
		{
			if(author_ids[i] == -1)
			{
				author_ids[i] = na::author_name_disambiguator
					.InferOrGenerateAuthorIdFromAuthorName(author_names[i], *publication);

				auto a2p = Author2pub(-1, author_ids[i], id, i+1);
				auto newAuthor = new Author(*GetAuthorById(author_ids[i]));
				newAuthor->pub_ids.push_back(id);
				UpdateAuthor(newAuthor);

				toUpdate.push_back(a2p);
			}
		}

		if(!toUpdate.empty())
		{
			UpdatePublication(publication);

			BackstageTasks::instance().addTask(new StatelessTask([toUpdate](){
				Author2pubDao author2pubDao;
				author2pubDao.batchInsert(toUpdate);
			}));

			stringstream message;
			message << "Fixed Author2Map relation for publication[id=" << id << "] and authors[id=";
			boost::function<int(const Author2pub& a2p)> to_aid = [](const Author2pub& a2p)->int{return a2p.aid;};
			copy(toUpdate | boost::adaptors::transformed(to_aid), 
				ostream_iterator<int>(message, ","));
			message << "].";
			LOG(INFO) << message.str();
		}

		return true;
	}

	namespace
	{
		inline bool isIndexedFieldChanged(const vector<const FieldInfo*>& updatedFields)
		{
			bool res = false;
			for (auto field : updatedFields)
			{
				res |= field->isIndexed;
			}
			return res;
		}
	}
	/*
	*	Update an existing publication
	*	
	*	Date: Dec 1st, 2011
	*	
	*	Input: mashaler::Publication
	*	Output: mashaler::string ("true" for success, otherwise "false")
	*/
	bool updatePublication( const string& input, string& output)
	{
		mashaler::Publication mPub;
		mPub.ParseFromString(input);

		DocumentCollection* pubs = IndexResource::instance().pubCollection();
		Publication* old_pub,* new_pub;
		if(!mPub.has_id() || (old_pub = (Publication*)pubs->getDocumentByIndex(mPub.id()))==NULL)
		{
			return false;
		}
		
		LOG(INFO) << boost::str(boost::format("[update]%d") % mPub.id());

		new_pub = new Publication(*old_pub);
		vector<const FieldInfo*> updatedFields;
		ProtoConverter::getUpdatedField(pubs, *new_pub, mPub, updatedFields);
		PublicationAdapter::instance().update(new_pub, updatedFields);

		if(isIndexedFieldChanged(updatedFields))
		{
			auto pub_index = IndexResource::instance().pubIndex();
			vector<int> ids;
			ids.push_back(new_pub->id);

			pub_index->removeDocumentIndex(ids);
			pub_index->addDocumentByIndex(ids);
		}
		output = "true";
		return true;
	}

	/*
	*	Remove existing publications with given ids
	*	
	*	Date: Dec 1st, 2011
	*	
	*	Input: mashaler::IntQueryParams
	*	Output: mashaler::string ("true" for success, otherwise "false")
	*/
	bool removePublications( const string& input, string& output)
	{
		mashaler::IntQueryParams param;
		param.ParseFromString(input);

		if(param.ids_size()>0)
		{
			IndexResource& resource = IndexResource::instance();
			auto pubs = resource.pubCollection(); 

			auto PubNotNULL = [&](int id)->bool{return pubs->getDocumentByIndex(id)!=NULL;};
			vector<int> ids;
			boost::range::copy(param.ids()|boost::adaptors::filtered(PubNotNULL), std::back_inserter(ids));

			LOG(INFO) << boost::str(boost::format("[remove]%s") % strutils::vectorToString(ids));

			if(!ids.empty())
			{
				PublicationAdapter::instance().remove(ids);
				resource.pubIndex()->removeDocumentIndex(ids);
			}
		}
		return true;
	}

	/*
	*	Reload publications from database, include updating index
	*	
	*	Date: Dec 1st, 2011
	*	
	*	Input: mashaler::IntQueryParams
	*	Output: mashaler::string ("true" for success, otherwise "false")
	*/
	bool reloadPublications( const string& input, string& output)
	{
		mashaler::IntQueryParams param;
		param.ParseFromString(input);

		if(param.ids_size()>0)
		{
			IndexResource& resource = IndexResource::instance();
			auto pubs = resource.pubCollection(); 

			auto PubNotNULL = [&](int id)->bool{return pubs->getDocumentByIndex(id)!=NULL;};
			vector<int> ids;
			boost::range::copy(param.ids()|boost::adaptors::filtered(PubNotNULL), std::back_inserter(ids));

			LOG(INFO) << boost::str(boost::format("[reload]%s") % strutils::vectorToString(ids));

			if(!ids.empty())
			{
				PublicationAdapter::instance().reload(ids);
				resource.pubIndex()->removeDocumentIndex(ids);
				resource.pubIndex()->addDocumentByIndex(ids);
			}
		}
		return true;
	}

	/*
	* Given an publication object, infer its authors' IDs using their names.
	* Original author IDs will be discarded.
	* Date: Nov 30, 2011
	*
	* Input: A publication object
	* Output: A list of authors' IDs.
	*/
	bool inferAuthorIdsOfPublication(const string& input, string& output)
	{
		mashaler::Publication m_publication;
		m_publication.ParseFromString(input);

		LOG(INFO) << boost::str(boost::format("[infer]%d %s") % m_publication.id() % m_publication.title());

		Publication publication(IndexResource::instance().pubCollection());
		ProtoConverter::convertPub(m_publication, publication);
		na::author_name_disambiguator.InferAuthorIdsFromAuthorNames(publication);

		mashaler::IntArray ids;
		boost::range::for_each(publication.authorIDs, [&](int id){ids.add_values(id);});

		return ids.SerializeToString(&output);
	}

	/*
	*	recommend a list of related publications using random walk algorithms
	*	
	*	Date: May 21st, 2012
	*	
	*	Input: mashaler::IntQueryParams
	*	Output: mashaler::PublicationResult
	*/
	bool recommendRelatedPaper(const string& input, string& output)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* publicationCollection = resource.pubCollection();
		
		if(params.ids_size()==0)
			return false;
		LOG(INFO) << boost::str(boost::format("[related]%d") % params.ids(0));
		
		mashaler::PublicationResult result;
		Document* doc;
		PublicationRecommender recommender;
		vector<int>&& ids = recommender.recommend(params.ids(0));
		result.set_total_count(ids.size());

		int max_index = min(params.offset() + params.count(), (int)ids.size());
		for(int i=params.offset(); i<max_index ;++i)
		{
			doc = publicationCollection->getDocumentByIndex(ids[i]);
			if(doc)
			{
				ProtoConverter::convertPub(resource.pubCollection(), *doc, *result.add_publications(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
	}
}
