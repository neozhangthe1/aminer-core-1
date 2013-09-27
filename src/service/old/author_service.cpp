#include "ExpertSearcher.h"
#include "AuthorService.h"
#include "IndexResource.h"
#include "AuthorAdapter.h"
#include "IndexResource.h"
#include "interface.pb.h"
#include "InterfaceUtils.h"
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include "AuthorUtils.h"
#include "stringutils.h"
#include "ProcessQuery.h"
#include "SocialGraphSearch.h"
#include "SearchConfigure.h"
#include "Enumerator.h"
#include <tuple>

namespace AuthorService
{
	using boost::range::copy;
	using std::tuple;

	bool searchAuthors(const string& input, string& output)//(string& query, int offset, int count, vector<string>& returnedFields)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);
		int totalCount = 0;

		LOG(INFO) << boost::str(boost::format("[search]%s") % params.query());

		string query = params.query();
		string orderby;
		ProcessQuery::get_param(query, "orderby", orderby);

		IndexResource& resource = IndexResource::instance();
		ExpertSearcher searcher(resource.pubIndex());
		DocumentCollection* authors = resource.authorCollection();
		QueryItemEnumerator* enumerator = searcher.search(params.query());

		mashaler::AuthorResult result;

		if(enumerator)
		{
			result.set_total_count(enumerator->size());
			QueryItem item;
			if(!orderby.empty())
			{
				QueryItemComparer* cmp = NULL;
				if(orderby=="pubcount")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::PUBCOUNT);
				}
				else if(orderby=="hindex")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::HINDEX);
				}
				else if(orderby=="activity")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::ACTIVITY);
				}
				else if(orderby=="longevity")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::LONGEVITY);
				}
				else if(orderby=="diversity")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::DIVERSITY);
				}
				else if(orderby=="sociability")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::SOCIABILITY);
				}
				else if(orderby=="gindex")
				{
					cmp = new AuthorFeatureComparer(AuthorFeatures::GINDEX);
				}
				
				if(cmp != NULL)
				{
					OrderedQueryItemEnumerator* new_enumer = new OrderedQueryItemEnumerator();
					for(int i=0;i<RERANK_SIZE && enumerator->next(item);++i)
					{
						new_enumer->addQueryItem(item);
					}
					new_enumer->setComparer(cmp);
					delete enumerator;
					enumerator = new_enumer;
				}
			}

			for(int i=0;i < params.offset() && enumerator->next(item); ++i);

			Document* doc;
			for(int i=0;i < params.count() && enumerator->next(item); ++i)
			{
				doc = authors->getDocumentByIndex(item.docId);
				if(doc)
					ProtoConverter::convertAuthor(authors , *(Author*)doc, *result.add_authors(), params.returned_fields());
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
	* Get author list by authors'id...(get one by one, skip/not return it if not found)
	* Author: kitty
	* Time:  2010/06/10
	*
	* Update by Hang Su <su_hang@live.com>
	* Date: June 17, 2011
	*/
	bool getAuthorsById(const string& input, string& output)//(vector<int>& naIds, vector<string>& returnedFields)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);
		//int totalCount = 0;
		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authors = resource.authorCollection();

		vector<int> ids;
		for(int i=0; i < params.ids_size(); i++)
		{
			int id = params.ids(i);
			ids.push_back(id);
		}
		LOG(INFO) << boost::str(boost::format("[id] %s") % strutils::vectorToString(ids));

		mashaler::AuthorResult result;
		Document* doc;
		result.set_total_count(authors->getDocCounts());
		for(int i=0; i < params.ids_size(); i++)
		{
			int onePubID = params.ids(i);

			doc = authors->getDocumentByIndex(onePubID);
			if(doc)
			{
				ProtoConverter::convertAuthor(authors , *(Author*)doc,  *result.add_authors(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
	}

	/*
	* Get author list by authors'name...(get one by one, skip/not return it if not found)
	* Author: kitty
	* Time:  2010/06/10
	*
	* Update by Hang Su <su_hang@live.com>
	* Date: June 17, 2011
	*
	*
	* Input: mashaler::StringQueryParams
	* Output: mashaler::AuthorResult
	*/
	bool getAuthorsByName(const string& input, string& output)//(string& name, vector<string>& returnedFields)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authorCollection = resource.authorCollection();
		Name2AuthorMap& name2Author = resource.name2AuthorMap();

		LOG(INFO) << boost::str(boost::format("[name]%s") % params.query());

		set<int>* authors = name2Author.getAuthorsByAuthorName(params.query());

		mashaler::AuthorResult result;
		if(authors)
		{	
			Document* doc;
			for(set<int>::iterator iter = authors->begin(); iter!=authors->end(); ++iter)
			{
				doc = authorCollection->getDocumentByIndex(*iter);
				if(doc)
					ProtoConverter::convertAuthor(authorCollection , *(Author*)doc, *result.add_authors(), params.returned_fields());
			}
			result.set_total_count(authors->size());
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}
	/*
	*Get all other authors with at least one same name to this author
	*
	*Author: Hang Su<su_hang@live.com> 
	*Date: July 14, 2011
	*
	* Input:	mashaler::IntQueryParams
	* Output:	mashaler::AuthorResult
	*/
	bool getAuthorsWithSameNameById(const string& input, string& output)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authorCollection = resource.authorCollection();
		Name2AuthorMap& name2Author = resource.name2AuthorMap();

		mashaler::AuthorResult result;

		LOG(INFO) << boost::str(boost::format("[same_name]%d") % params.ids(0));

		Author* author = (Author*)resource.authorCollection()->getDocumentByIndex(params.ids(0));

		if(author)
		{
			set<int> author_set;
			set<int>* authors;
			for (string& name : author->names)
			{
				authors = name2Author.getAuthorsByAuthorName(name);
				if(authors)
					author_set.insert(authors->begin(), authors->end());
			}
			vector<Author*> author_vector;
			author_vector.reserve(author_set.size());
			for (int author_id : author_set)
			{
				author = (Author*)authorCollection->getDocumentByIndex(author_id);
				if(!author)
					continue;
				author_vector.push_back(author);
			}
			sort(author_vector.begin(), author_vector.end(), AuthorUtils::compare_hindex_pub);

			Document* doc;
			for(vector<Author*>::iterator iter = author_vector.begin(); iter!=author_vector.end(); ++iter)
			{
				doc = *iter;
				ProtoConverter::convertAuthor(authorCollection , *(Author*)doc, *result.add_authors(), params.returned_fields());
			}
			result.set_total_count(author_vector.size());
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Get all authors with at least one same name to the authors got by the given name
	*
	*Author: Hang Su<su_hang@live.com> 
	*Date: July 14, 2011
	*
	* Input:	mashaler::StringQueryParams
	* Output:	mashaler::AuthorResult
	*/
	bool getAuthorsWithSameNameByName(const string& input, string& output)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authorCollection = resource.authorCollection();
		Name2AuthorMap& name2Author = resource.name2AuthorMap();

		LOG(INFO) << boost::str(boost::format("[same_name] %s") % params.query());

		mashaler::AuthorResult result;
		set<int>* authors = name2Author.getAuthorsByAuthorName(params .query());
		Author* author;
		if(authors)
		{
			set<int> author_set;
			set<int>* new_authors;
			for (int author_id : *authors)
			{
				author = (Author*)authorCollection->getDocumentByIndex(author_id);
				if(!author)
					continue;
				for (string& name : author->names)
				{
					new_authors = name2Author.getAuthorsByAuthorName(name);
					if(new_authors)
						author_set.insert(new_authors->begin(), new_authors->end());
				}
			}

			vector<Author*> author_vector;
			author_vector.reserve(author_set.size());
			for (int author_id : author_set)
			{
				author = (Author*)authorCollection->getDocumentByIndex(author_id);
				if(!author)
					continue;
				author_vector.push_back(author);
			}
			sort(author_vector.begin(), author_vector.end(), AuthorUtils::compare_hindex_pub);

			Document* doc;
			for(vector<Author*>::iterator iter = author_vector.begin(); iter!=author_vector.end(); ++iter)
			{
				doc = *iter;
				ProtoConverter::convertAuthor(authorCollection , *(Author*)doc, *result.add_authors(), params.returned_fields());
			}
			result.set_total_count(author_vector.size());
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Add a new author if its naid not specified, or update an existing one if otherwise.
	*
	*Date: Oct 12, 2011
	*
	* Input:	mashaler::Author
	* Output:	std::string ("true" if succeeded, otherwise "false")
	*/
	bool addUpdateAuthor( const string& input, string& output )
	{
		mashaler::Author mAuthor;
		mAuthor.ParseFromString(input);

		LOG(INFO) << boost::str(boost::format("[update]%d") % mAuthor.naid());

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authorCollection = resource.authorCollection();
		Author* author_ptr = NULL;
		vector<const FieldInfo*> affectedFields;
		ProtoConverter::getUpdatedOrCreateNewAuthorWithMashaler(mAuthor, authorCollection, author_ptr, affectedFields);
		if(author_ptr->naid!= -1)
			AuthorAdapter::instance().update(author_ptr,affectedFields);
		else
			AuthorAdapter::instance().add(author_ptr);
		output = "true";
		return true;
	}

	/*	Merge authors into one, move corresponding pubs
	*
	*	Date: August 31, 2011
	*	
	*	Input:	mashaler::MergeAuthorParam
	*	Output: string
	*/
	bool mergeAuthor(const string& input, string& output)
	{
		mashaler::MergeAuthorParam param;
		param.ParseFromString(input);

		AuthorAdapter& adapter = AuthorAdapter::instance();
		vector<int> naids;
		for(int i = 0 ;i<param.naids_size();++i)
			naids.push_back(param.naids(i));

		LOG(INFO) << boost::str(boost::format("[merge] %s") % strutils::vectorToString(naids));

		adapter.merge(naids, string(param.names()), param.contact_id(), Author::TYPE_COMBINED_MANULLY); 
		output="true";
		return true;
	}

	/*	Fack merge authors into one, used for counting the objects that would be affected
	*
	*	Date: August 31, 2011
	*	
	*	Input:	mashaler::MergeAuthorParam
	*	Output: mashaler::StringDoublePairs
	*/
	bool fakeMergeAuthor(const string& input, string& output)
	{
		mashaler::MergeAuthorParam param;
		param.ParseFromString(input);


		int update_author_count = 0;
		int update_a2p_count = 0;
		int delete_author_count = 0;

		if(param.naids_size())
			update_author_count = 1;

		DocumentCollection* authors = IndexResource::instance().authorCollection();
		Author* author;
		for(int i = 1 ;i<param.naids_size();++i)
		{
			author = (Author*)authors->getDocumentByIndex(param.naids(i));
			if(author)
			{
				update_a2p_count+= author->pub_ids.size();
				delete_author_count++;
			}
		}
		mashaler::StringDoublePairs result;
		mashaler::StringDoublePair* pair;
		pair = result.add_items();
		pair->set_key("update na_person");
		pair->set_value(update_author_count);

		pair = result.add_items();
		pair->set_key("update na_author2pub");
		pair->set_value(update_a2p_count);

		pair = result.add_items();
		pair->set_key("delete from na_person");
		pair->set_value(delete_author_count);
		return result.SerializeToString(&output);
	}

	/*	Merge authors into one, move corresponding pubs
	*
	*	Date: August 31, 2011
	*	
	*	Input:	mashaler::RelationSearchParam
	*	Output: string
	*/
	bool searchAuthorRelation(const string& input, string& output)
	{
		mashaler::RelationSearchParam param;
		param.ParseFromString(input);

		vector<int> person_ids, expand_ids, remove_ids;
		int algo = param.algo();
		ProtoConverter::convert(param.person_ids(), person_ids);
		ProtoConverter::convert(param.expand_ids(), expand_ids);
		ProtoConverter::convert(param.remove_ids(), remove_ids);

		LOG(INFO) << boost::str(boost::format("[search_rel] person:[%s], expand:[%s], remove:[%s]") % strutils::vectorToString(person_ids)  % strutils::vectorToString(expand_ids)  % strutils::vectorToString(remove_ids));

		SocialGraphSearch search(person_ids, expand_ids, remove_ids, algo);
		vector<AuthorRelation> relations = search.getSocialNetwork();

		mashaler::AuthorRelations result;
		for (AuthorRelation& relation : relations)
		{
			ProtoConverter::convert(relation, *result.add_relations());
		}
		return result.SerializeToString(&output);
	}

	/*	Merge authors into one, move corresponding pubs
	*
	*	Date: August 31, 2011
	*	
	*	Input:	mashaler::MovePubParam
	*	Output: string
	*/
	bool movePublication(const string& input, string& output)
	{
		mashaler::MovePubParam param;
		param.ParseFromString(input);

		LOG(INFO) << boost::str(boost::format("[move_pub] move %d from %d to %d") % param.pub_id() % param.from() % param.to());

		AuthorAdapter::instance().move(param.from(), param.to(), param.pub_id());
		output = "true";
		return true;
	}

	/*	Remove the authors with given naids
	*
	*	Date: Nov. 10, 2011
	*
	*	Input: mashaler::IntQueryParams
	*	Output: string{"true" for success, "false" for fail}
	*/
	bool removeAuthors(const string& input, string& output)
	{
		mashaler::IntQueryParams param;
		param.ParseFromString(input);

		AuthorAdapter& adapter = AuthorAdapter::instance();
		IndexList* author_index = IndexResource::instance().authorIndex();

		vector<int> ids;
		boost::range::copy(param.ids(), std::back_inserter(ids));

		LOG(INFO) << boost::str(boost::format("[remove]%s") % strutils::vectorToString(ids));

		author_index->removeDocumentIndex(ids);
		adapter.remove(ids);
		output="true";
		return true;
	}

	/*
	* Search author profile 
	* Author: Hang Su<su_hang@live.com> 
	* Date: Dec 28, 2011
	*
	* Input: mashaler::StringQueryParams
	* Output: mashaler::PublicationScoredResult
	*/
	bool searchAuthorProfile(const string& input, string& output)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		LOG(INFO) << boost::str(boost::format("[search_profile]%s") % params.query());

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authors = resource.authorCollection();

		IndexList* index = resource.authorIndex();
		Searcher searcher(index, new StandardQueryAnalyzer(index));
		searcher.SetSearchedFields(SearchConfigure::instance().authorDefSearchFields);
		QueryItemEnumerator* enumerator = searcher.Search(params.query());

		mashaler::AuthorResult result;

		if(enumerator)
		{
			result.set_total_count(enumerator->size());
			QueryItem item;

			for(int i=0;i < params.offset() && enumerator->next(item); ++i);

			Document* doc;
			for(int i=0;i < params.count() && enumerator->next(item); ++i)
			{
				doc = authors->getDocumentByIndex(item.docId);
				if(doc)
					ProtoConverter::convertAuthor(authors , *(Author*)doc, *result.add_authors(), params.returned_fields());
			}
			delete enumerator;
		}
		else
		{
			result.set_total_count(0);
		}

		return result.SerializeToString(&output);
	}

	namespace
	{
		int edit_dist(string vStrRow, string vStrColumn)
		{
			boost::to_lower(vStrRow);
			boost::to_lower(vStrColumn);

			int row = vStrColumn.length();  
			int column = vStrRow.length();
			const int sizeR = row + 1;
			const int sizeC = column + 1;

			if(row>99)
				row = 99;
			if(column>99)
				column = 99;

			int pScore[100][100];


			//初始化第一行和第一列
			for (int c = 0; c <= column; c++)
				pScore[0][c] = c;
			for (int r = 0; r <= row; r++)
				pScore[r][0] = r;



			//从v(1,1)开始每列计算
			for (int c = 1; c <= column; c++)
			{
				for (int r = 1; r <= row; r++)
				{
					//计算v(i,j)

					int valueMatch;
					if (vStrColumn[r-1] == vStrRow[c-1])
						valueMatch = 0;
					else
						valueMatch = 1;  
					pScore[r][c] = min(min(pScore[r-1][c] + 1, pScore[r][c-1] + 1),pScore[r-1][c-1] + valueMatch);
				}
			}

			return pScore[row][column];
		}
	}

	/*
	* get query suggest for searching author name
	* Author: Hang Su<su_hang@live.com> 
	* Date: Nov 28, 2011
	*
	* Input: string(the original query)
	* Output: string(the suggested author name)
	*/
	bool authorSearchSuggest(const string& input, string& output)
	{
		IndexResource& resource = IndexResource::instance();
		DocumentCollection* authors = resource.authorCollection();

		LOG(INFO) << boost::str(boost::format("[search_suggest] %s") % input);

		auto author_set = resource.name2AuthorMap().getAuthorsByAuthorName(input);
		if(author_set)
		{
			output.clear();
			return true;
		}

		Author* author;
		IndexList* index = resource.authorIndex();
		Searcher searcher(index, new StandardQueryAnalyzer(index));
		searcher.SetSearchedFields(SearchConfigure::instance().authorDefSearchFields);
		QueryItemEnumerator* enumerator = searcher.Search(input);

		if(enumerator)
		{

			QueryItem item;
			int max_count = 1000;
			typedef std::tuple<int,int,string> triple;
			vector<triple> tuples;
			tuples.reserve(2000);

			while(enumerator->next(item) && max_count--)
			{
				author = (Author*)authors->getDocumentByIndex(item.docId);
				if(author)
				{
					for (string& name : author->names)
					{
						tuples.push_back(make_tuple(edit_dist(name, input), -1*author->h_index, name));						
					}
				}
			}
			delete enumerator;
			if(tuples.size()>3)
			{
				if(tuples.size()>30)
					std::partial_sort(tuples.begin(), tuples.begin()+30, tuples.end());//,[](triple& left,triple& right)->bool{return left<right;});
				else
					std::sort(tuples.begin(), tuples.end());

				int dist = get<0>(tuples[0]);
				auto iter = tuples.begin()+1;
				output = get<2>(tuples[0]);
				int counter = 2;

				set<string> added;
				string add = output;
				boost::to_lower(add);
				added.insert(add);

				while(counter && iter!=tuples.end() && get<0>(*iter)==dist)
				{
					add = get<2>(*iter);
					boost::to_lower(add);
					if(added.find(add)==added.end())
					{
						added.insert(add);
						output+="|";
						output+= get<2>(*iter);
						--counter;
					}
					++iter;
				}
			}
			else if(!tuples.empty())
			{
				auto ptr = std::min_element(tuples.begin(), tuples.end());
				output = get<2>(*ptr);
			}

		}
		else
		{
			output.clear();
		}
		return true;
	}

	/*
	* reload authors with given naids
	* Author: Hang Su<su_hang@live.com> 
	* Date: Dec 1, 2011
	*
	* Input: mashaler::IntQueryParams
	* Output: string(status)
	*/
	bool reloadAuthors(const string& input, string& output)
	{
		mashaler::IntQueryParams param;
		param.ParseFromString(input);

		vector<int> naids;
		boost::range::copy(param.ids(), std::back_inserter(naids));

		LOG(INFO) << boost::str(boost::format("[reload] %s") % strutils::vectorToString(naids));

		AuthorAdapter::instance().reload(naids);

		IndexList* author_index = IndexResource::instance().authorIndex();
		author_index->removeDocumentIndex(naids);
		author_index->addDocumentByIndex(naids);

		output = "true";
		return true;
	}

	/*
	* recalculate statistic features of given authors
	* Author: Hang Su<su_hang@live.com> 
	* Date: Dec 1, 2011
	*
	* Input: mashaler::IntQueryParams
	* Output: string(status)
	*/
	bool recalcuFeatures(const string& input, string& output)
	{
		mashaler::IntQueryParams param;
		param.ParseFromString(input);
		
		vector<int> naids;
		boost::range::copy(param.ids(), std::back_inserter(naids));

		LOG(INFO) << boost::str(boost::format("[recalcu_feature] %s") % strutils::vectorToString(naids));

		boost::range::for_each(param.ids(), [](int naid){
			AuthorAdapter::instance().recalcufeature(naid);
		});
		output = "true";
		return true;
	}
}
