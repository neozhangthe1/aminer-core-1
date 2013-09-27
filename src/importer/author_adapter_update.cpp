#include "author_adapter.h"
#include "Author2ContactDao.h"
#include "ContactInfoAdapter.h"
#include "AuthorInterestDao.h"
#include "AuthorFeatureDao.h"
#include "AuthorRelationDao.h"
#include "OrganizationAdapter.h"
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include "document.h"
#include "stringutils.h"
#include "interface.pb.h"
#include "PublicationAdapter.h"
#include "MemGC.h"
#include "AuthorTask.h"
#include "Name2AuthorMap.h"
#include "NaidMap.h"
#include "Author2pubDao.h"
#include <unordered_set>
#include <boost/algorithm/string.hpp>

void AuthorAdapter::reload(vector<int>& naids)
{
	Author2ContactDao a2cDao;
	ContactInfoAdapter& contactAdapter = ContactInfoAdapter::instance();
	AuthorInterestDao interestDao;
	AuthorFeatureDao featureDao;
	AuthorRelationDao relationDao;
	Author2OrgDao orgDao;
	Author2pubDao pubDao;
	DocumentCollection* pubs = PublicationAdapter::instance().getDocCollection();

	for (int naid : naids)
	{
		NapersonItem&& item = a2cDao.getItem(naid);
		if(item.naid == -1)
		{
			continue;
		}
		Author* author = new Author(NULL);
		author->names = item.names;
		author->naid = naid;
		author->type = static_cast<Author::Type>(item.type);
		author->contact_id = item.contactid;

		if(author->contact_id != -1)
		{

			ContactInfo* info = contactAdapter.getItem(item.contactid);
			if(info)
			{
				attach_contact_info(author, info);
			}
		}

		auto&& items = interestDao.getAuthorInterest(naid);
		boost::range::for_each(items, [&](const InterestItem& item)->void{
			if(item.year==1 || item.year==-1)
			{
				if(author->interest.empty() || item.year==-1)
				{
					author->interest.assign(item.interest);
				}
			}
			else
			{
				if(item.year>0)
				{
					author->interest_by_year.push_back(item.interest);
					author->interest_years.push_back(item.year);
				}
				else
				{
					for(int index = 0;index<author->interest_years.size();++index)
					{
						if(author->interest_years[index] == item.year*-1)
						{
							author->interest_by_year[index] = item.interest;
						}
					}
				}
			}
		});

		featureDao.getAuthorFeature(*author);

		author->Coauthor = relationDao.getRelations(naid);

		author->org_id = orgDao.getAuthorOrg(naid);

		author->assign_statistics();

		vector<int>&& pub_ids = pubDao.getAuthorPubs(naid);

		copy_if(pub_ids.begin(), pub_ids.end(), std::back_inserter(author->pub_ids), [&](int pub_id)->bool{ if(pubs->getDocumentByIndex(pub_id)==NULL)return false; else return true;});

		this->collection->assign(naid, author, false);
	}
}

void AuthorAdapter::update(Author* author, vector<const FieldInfo*>& updated_fields)
{
	if(!author || updated_fields.empty())
		return;
	std::lock_guard<std::mutex> guard(this->update_mutex);
	int change = get_change(updated_fields);
	if(change & CONTACT_INFO_CHANGED)
	{
		ContactInfo contact_info;
		get_contact_info(*author, contact_info);
		if(author->contact_id == -1)
		{//insert contact_info first
			author->contact_id = ContactInfoAdapter::instance().insert(contact_info);
			BackstageTasks::instance().addTask(new UpdateAuthor2ContactTask(author->naid, string(), author->contact_id, -1));
		}
		else
		{//update contact_info TODO backstage task
			BackstageTasks::instance().addTask(new UpdateAuthorContactInfoTask(contact_info));
		}
	}
	if(change & INTEREST_CHANGED)
	{
		BackstageTasks::instance().addTask(new UpdateAuthorInterestTask(author->naid, author->interest, -1));
	}
	if(change & (NAMES_CHANGED | TYPE_CHANGED))
	{
		BackstageTasks::instance().addTask(new UpdateAuthor2ContactTask(author->naid, strutils::join(author->names, ","), author->contact_id, author->type));
	}
	this->collection->assign(author->naid, author, false);
}

int AuthorAdapter::add(Author* author)
{
	std::lock_guard<std::mutex> guard(this->update_mutex);
	Author2ContactDao dao;
	int naid = dao.addItem(strutils::join(author->names, ","), author->contact_id, author->type);
	author->naid = naid;
	this->collection->assign(naid, author, false);
	Name2AuthorMap::instance().add(author);
	return naid;
}

void AuthorAdapter::remove(vector<int>& naids)
{
	std::lock_guard<std::mutex> guard(this->update_mutex);
	boost::range::for_each(naids, [&](int naid){
		Author* author = (Author*)this->collection->getDocumentByIndex(naid);
		if(author)
		{
			this->collection->assign(naid, NULL, false);
			Name2AuthorMap::instance().remove(author);
		}
	});

	auto ids = shared_ptr<vector<int>>(new vector<int>(naids));
	BackstageTasks::instance().addTask(new StatelessTask([ids](){
		Author2ContactDao dao;
		dao.removeItem(*ids);
	}));
}

void AuthorAdapter::recalcufeature(int naid)
{
	Author* author = (Author*)this->collection->getDocumentByIndex(naid);
	if(!author)
		return;
	Author* new_author = new Author(*author);
	FeatureCaculator::calcu_features(*new_author);
	this->collection->assign(naid, new_author, false);
	BackstageTasks::instance().addTask(new StatelessTask([naid, this](){
		Author* author = (Author*)AuthorAdapter::instance().getDocCollection()->getDocumentByIndex(naid);
		if(author)
		{
			AuthorFeatureDao dao;
			dao.updateAuthorFeatureInDB(*author);
		}
	}));
}

int AuthorAdapter::get_change(vector<const FieldInfo*>& updated_fields)
{
	int result = 0;
	for (const FieldInfo* field : updated_fields)
	{
		if(field->name != "naid")
		{
			if(field->name == "names")
			{
				result |= NAMES_CHANGED;
			}
			else if(field->name == "type")
			{
				result |= TYPE_CHANGED;
			}
			else if(field->name == "interest")
			{
				result |= INTEREST_CHANGED;
			}
			else
			{
				result |= CONTACT_INFO_CHANGED;
			}
			if(result & NAMES_CHANGED & INTEREST_CHANGED & CONTACT_INFO_CHANGED & TYPE_CHANGED)
				break;
		}
	}
	return result;
}

bool AuthorAdapter::get_contact_info(Author& author, ContactInfo& contact_info)
{
	contact_info.email = author.email;
	contact_info.address = author.address;
	contact_info.phone = author.phone;
	contact_info.fax = author.fax;
	contact_info.homepage = author.homepage;
	contact_info.position = author.position;
	contact_info.affiliation = author.affiliation;
	contact_info.imgurl = author.imgurl;
	contact_info.imgname = author.imgname;
	contact_info.imgsrc = author.imgsrc;
	contact_info.phduniv = author.phduniv;
	contact_info.phdmajor = author.phdmajor;
	contact_info.phddate = author.phddate;
	contact_info.msuniv = author.msuniv;
	contact_info.msmajor = author.msmajor;
	contact_info.msdate = author.msdate;
	contact_info.bsuniv = author.bsuniv;
	contact_info.bsmajor = author.bsmajor;
	contact_info.bsdate = author.bsdate;
	contact_info.bio = author.bio;
	contact_info.contactId = author.contact_id;
	return true;
}

void AuthorAdapter::move(int from, int to, int pub_id, int update)
{
	std::lock_guard<std::mutex> guard(this->update_mutex);
	DocumentCollection* pubCollection = PublicationAdapter::instance().getDocCollection();
	Publication* pub = (Publication*)pubCollection->getDocumentByIndex(pub_id);

	if(pub==NULL)
		return;

	Author* author_from = (Author*)this->collection->getDocumentByIndex(from);
	Author* author_to = (Author*)this->collection->getDocumentByIndex(to);

	if(author_from == NULL || author_to==NULL)
		return;

	bool flag = false;
	int index =0;
	for(; index<author_from->pub_ids.size(); ++index)
	{
		if(author_from->pub_ids[index] == pub_id)
		{
			flag = true;
			break;
		}
	}
	if(flag)
	{
		Author* new_author_from = new Author(*author_from);
		Author* new_author_to = new Author(*author_to);
		Publication* new_pub = new Publication(*pub);

		new_author_from->pub_ids.erase(new_author_from->pub_ids.begin()+index);
		new_author_to->pub_ids.push_back(new_pub->id);

		for(int i=0;i<new_pub->authorIDs.size(); ++i)
		{
			if(new_pub->authorIDs[i]==from)
			{
				new_pub->authorIDs[i] = to;
				break;
			}
		}
		//TODO caculate new feature value & rank
        //XXX why '&&' not '&'?
		if(update && UPDATE_FEATURE)
		{
			FeatureCaculator::calcu_features(*new_author_from);
			FeatureCaculator::calcu_features(*new_author_to);
		}

		this->collection->assign(from, new_author_from, false);
		this->collection->assign(to, new_author_to, false);
		pubCollection->assign(pub_id, new_pub, false);

		//asynchronous write back to the database
		BackstageTasks::instance().addTask(new MovePubTask(from, to, pub_id));
	}
}

void AuthorAdapter::merge(vector<int>& ids, const string& name, int contact_id, Author::Type type, int update)
{
	if(ids.empty())
		return;
	std::lock_guard<std::mutex> guard(this->update_mutex);
	Author *to, *from, *original_to;
	original_to = to = (Author*)this->collection->getDocumentByIndex(ids[0]);

	if(!to || ids.size()<2)
		return;

	to = new Author(*to);
	to->type = type;
	vector<Author*> authors;
	authors.reserve(ids.size());
	authors.push_back(to);

	vector<Author2pub> moved_pubs;
	vector<int> merged_ids;
	for(int i=1;i<ids.size(); ++i)
	{
		from = (Author*)this->collection->getDocumentByIndex(ids[i]);
		if(!from)
			continue;
		this->collection->assign(ids[i], NULL, false);
		const vector<Author2pub>& temp = move_all_pubs(from, to);
		moved_pubs.insert(moved_pubs.end(), temp.begin(), temp.end());
		authors.push_back(from);
		merged_ids.push_back(ids[i]);
	}

	//handle contact_info
	if(contact_id!=-1 && to->contact_id!=contact_id)
	{
		ContactInfo* info = ContactInfoAdapter::instance().getItem(contact_id);
		if(info)
			attach_contact_info(to, info);
	}
	else
	{
		Author* contact_src = choose_contact(authors);
		if(contact_src != to)
			copy_contactinfo(contact_src, to);
	}

	//handle names
	if(name.length())
	{
		to->names.clear();
		boost::split(to->names, name, boost::is_any_of(","), boost::token_compress_on);
	}
	else
	{
		this->merge_names(authors);
	}

	authors.push_back(original_to);
	Name2AuthorMap::instance().merge(authors);
	NaidMap::instance().add(merged_ids, ids[0]);
    //XXX why '&&' not '&'?
	if(update && UPDATE_FEATURE)
	{
		FeatureCaculator::calcu_features(*to);
	}
	this->collection->assign(to->naid, to,false);
	BackstageTasks::instance().addTask(new MergeAuthorTask(merged_ids, ids[0], strutils::join(to->names, ","),to->contact_id, type, moved_pubs));
}

Author* AuthorAdapter::choose_contact(vector<Author*>& authors)
{
	Author* res = 0;
	int max = -1;
	for (Author* author : authors)
	{
		int temp = 0;
		if(author->email.length())
			++temp;
		if(author->position.length())
			++temp;
		if(author->affiliation.length())
			++temp;
		if(author->address.length())
			++temp;
		if(author->phone.length())
			++temp;
		if(author->fax.length())
			++temp;
		if(author->homepage.length())
			++temp;

		if(temp>max)
		{
			max = temp;
			res = author;
		}
	}

	return res;
}

void AuthorAdapter::copy_contactinfo(Author* from, Author* to)
{
	if(from==to)
		return;
	to->contact_id = from->contact_id;
	to->email = from->email;
	to->address = from->address;
	to->phone = from->phone;
	to->fax = from->fax;
	to->homepage = from->homepage;
	to->position = from->position;
	to->affiliation = from->affiliation;
	to->imgurl = from->imgurl;
	to->phduniv = from->phduniv;
	to->phdmajor = from->phdmajor;
	to->phddate = from->phddate;
	to->msuniv = from->msuniv;
	to->msmajor = from->msmajor;
	to->msdate = from->msdate;
	to->bsuniv = from->bsuniv;
	to->bsmajor = from->bsmajor;
	to->bsdate = from->bsdate;
	to->bio = from->bio;
}

vector<Author2pub> AuthorAdapter::move_all_pubs(Author* from, Author* to)
{
	vector<Author2pub> res;
	if(!(from&&to))
		return res;
	DocumentCollection* pubCollection = PublicationAdapter::instance().getDocCollection();
	Publication* pub, *new_pub;
	for (int pub_id : from->pub_ids)
	{
		pub = (Publication*)pubCollection->getDocumentByIndex(pub_id);
		if(!pub)
			continue;
		new_pub = new Publication(*pub);
		for(int i=0;i<new_pub->authorIDs.size();++i)
		{
			if(new_pub->authorIDs[i]==from->naid)
			{
				new_pub->authorIDs[i]=to->naid;
				res.push_back(Author2pub(-1, from->naid, pub->id, -1));
			}
		}
		pubCollection->assign(pub->id, new_pub, false);		
		to->pub_ids.push_back(pub_id);
	}
	return res;
}

void AuthorAdapter::merge_names(vector<Author*>& authors)
{
    unordered_set<string> names;
	names.insert(authors[0]->names.begin(), authors[0]->names.end());
	for(int i=1;i<authors.size();++i)
	{
		for (string& name : authors[i]->names)
		{
			if(names.find(name)==names.end())
			{
				names.insert(name);
				authors[0]->names.push_back(name);
			}
		}
	}
}

//Attach and delete the contact_info
void AuthorAdapter::attach_contact_info(Author* author, ContactInfo* info)
{
	//author->naid = naid;
	//author->name = info->name;
	author->email = info->email;
	author->address = info->address;
	author->phone = info->phone;
	author->fax = info->fax;
	author->homepage = info->homepage;
	author->position = info->position;
	author->affiliation = info->affiliation;
	author->imgurl = info->imgurl;
	author->imgname = info->imgname;
	author->imgsrc = info->imgsrc;
	author->phduniv = info->phduniv;
	author->phdmajor = info->phdmajor;
	author->phddate = info->phddate;
	author->msuniv = info->msuniv;
	author->msmajor = info->msmajor;
	author->msdate = info->msdate;
	author->bsuniv = info->bsuniv;
	author->bsmajor = info->bsmajor;
	author->bsdate = info->bsdate;
	author->bio = info->bio;
	delete info;
}
