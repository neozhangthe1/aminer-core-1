#include "IndexResource.h"
#include "PublicationAdapter.h"
#include "AuthorAdapter.h"
#include "JConfMap.h"
#include "OrganizationAdapter.h"
#include "ACTadapter.h"
#include "Logger.h"
#include "util/threads.h"

using namespace std;
using namespace std::placeholders;

IndexResource& IndexResource::instance()
{
	static IndexResource instance;
	return instance;
}

IndexList* IndexResource::pubIndex()
{
	return _pubIndex;
}

IndexList* IndexResource::authorIndex()
{
	return _authorIndex;
}

DocumentCollection* IndexResource::orgCollection()
{
	return _orgCollection;
}

DocumentCollection* IndexResource::pubCollection()
{
	return _pubCollection;
}

DocumentCollection* IndexResource::authorCollection()
{
	return _authorCollection;
}

Name2AuthorMap& IndexResource::name2AuthorMap()
{
	return this->_name2AuthorMap;
}

IndexResource::IndexResource()
	:_name2AuthorMap(Name2AuthorMap::instance())
{


    ACTadapter::getInstance();

	LOG(INFO) << "loading organization collection";
	OrganizationAdapter& orgAdapter = OrganizationAdapter::instance();
	_orgCollection = orgAdapter.collection;

	LOG(INFO) << "loading publication collection...";
	PublicationAdapter& pubAdapter = PublicationAdapter::instance();
	_pubCollection = pubAdapter.getDocCollection();

	LOG(INFO) << "Assigning publication topic...";
    dispatch_thread_group(std::bind(&IndexResource::confirmPubTopic, this, std::placeholders::_1, 16), 16);

	LOG(INFO) << "loading author collection...";
	AuthorAdapter& authorAdapter = AuthorAdapter::instance();
	_authorCollection = authorAdapter.getDocCollection();

	LOG(INFO) << "building author index";
	_authorIndex = new IndexList(_authorCollection);
	_authorIndex->build();

	LOG(INFO) << "building name to authors map...";
	_name2AuthorMap.init(this->_authorCollection);

	LOG(INFO) << "Assigning publication to conference and author...";
	assignPub2ConfAndAuthor();

	LOG(INFO) << "building pub index...";
	_pubIndex = new IndexList(_pubCollection);
	_pubIndex->build();
}

void IndexResource::assignPub2ConfAndAuthor()
{
	Publication* pub = NULL;
	JournalConferenceInfo* conf = NULL;
	Author* author = NULL;
	Organization* org = NULL;

	int total = this->_pubCollection->getDocCounts();
	JConfMap& jconfMap  = *(JConfMap::getInstance());
	for(int i=0;i< total;++i)
	{
		pub = (Publication*) this->_pubCollection->getDocumentByIndex(i);
		if(pub)
		{
			conf = jconfMap.getJConf(pub->confID);
			if(conf)
			{
				conf->pub_ids.push_back(i);
			}

			for(vector<int>::iterator iter=pub->authorIDs.begin(); iter!= pub->authorIDs.end();++iter)
			{
				author = (Author*) this->_authorCollection->getDocumentByIndex(*iter);
				if(author)
				{
					author->pub_ids.push_back(i);
				}
			}

			for (int org_id : pub->org_ids)
			{
				org = (Organization*) this->_orgCollection->getDocumentByIndex(org_id);
				if(org)
				{
					org->pub_ids.push_back(i);
				}
			}
		}
	}
}

void IndexResource::confirmPubTopic(int part_id, int part_count)
{
	Publication* pub = NULL;

	int total = this->_pubCollection->getDocCounts();
	for(int i=part_id;i< total;i+=part_count)
	{
		pub = (Publication*) this->_pubCollection->getDocumentByIndex(i);
		if(pub)
		{
			pub->confirm_topic();
		}
		if(i%40000==0)
			LOG(INFO) << boost::str(boost::format("Assigning topic for pub %d") % i);
	}
}
