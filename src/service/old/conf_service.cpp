#include "ConfService.h"
#include "search.h"
#include "IndexResource.h"
#include "ConferenceSearcher.h"
#include "SearchConfigure.h"
#include "JConfMap.h"
#include "Timer.h"
#include "InterfaceUtils.h"
#include "interface.pb.h"

namespace ConfUtils
{
	inline bool compare_with_name(JournalConferenceInfo* left, JournalConferenceInfo* right)
	{
		return left->name<right->name;
	}

	inline bool compare_with_pub_count(JournalConferenceInfo* left, JournalConferenceInfo* right)
	{
		return left->pub_ids.size() > right->pub_ids.size();
	}

	inline bool compare_with_score(JournalConferenceInfo* left, JournalConferenceInfo* right)
	{
		return left->score > right->score;
	}
}

namespace ConfService
{
	/*
	* Input: mashaler::StringQueryParams
	* Output: mashaler::ConferenceResult
	*/
	bool searchConferences(const string& input, string& output)//(string& query, int offset, int count)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		LOG(INFO) << boost::str(boost::format("[search] %s") % params.query());

		IndexResource& resource = IndexResource::instance();
		JConfMap* jConfMapInstance = JConfMap::getInstance();
		ConferenceSearcher searcher(resource.pubIndex());
		mashaler::ConferenceResult result;

		JournalConferenceInfo* journalConferenceInfo;

		QueryItemEnumerator* enumerator = searcher.search(params.query());

		if(enumerator)
		{
			result.set_total_count(enumerator->size());
			QueryItem item;

			for(int i=0;i < params.offset() && enumerator->next(item); ++i);


			for(int i=0;i < params.count() && enumerator->next(item); ++i)
			{
				journalConferenceInfo = jConfMapInstance->getJConf(item.docId);
				//assert(journalConferenceInfo!=NULL);
				if(journalConferenceInfo)
					ProtoConverter::convert(*journalConferenceInfo, *result.add_confs());
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
	* Get one conf. by confName...
	* Author: kitty
	* Time:  2010/06/10
	*
	* Update by Hang Su <su_hang@live.com>
	* Date: June 17, 2011
	*
	*
	* Input: string 
	* Output: mashaler::PublicationResult
	*/
	bool getConferenceByName(const string& input, string& output)
	{
		JConfMap* jConfMapInstance = JConfMap::getInstance();
		JournalConferenceInfo* journalConferenceInfo;
		
		LOG(INFO) << boost::str(boost::format("[get] %s") % input);

		int jconfID = jConfMapInstance->getIdByName(input);
		journalConferenceInfo =jConfMapInstance->getJConf(jconfID);

		mashaler::ConferenceResult result;

		if(journalConferenceInfo)
		{
			ProtoConverter::convert(*journalConferenceInfo, *result.add_confs());
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	bool getConferenceById(const string& input, string& output)
	{
		JConfMap* jConfMapInstance = JConfMap::getInstance();
		JournalConferenceInfo* journalConferenceInfo;
		int confId;

		LOG(INFO) << boost::str(boost::format("[get] %s") % input);

		confId = atoi(input.c_str());
		journalConferenceInfo =jConfMapInstance->getJConf(confId);

		mashaler::ConferenceResult result;

		if(journalConferenceInfo)
		{
			ProtoConverter::convert(*journalConferenceInfo, *result.add_confs());
		}
		else
		{
			result.set_total_count(0);
		}
		return result.SerializeToString(&output);
	}

	/*
	* Get conferences start with the given prefix, with alphabetical order of name
	* Author:Hang Su <su_hang@live.com>
	* Time:  2011/09/08
	*
	* Input: string 
	* Output: mashaler::ConferenceScoredResult
	*/
	bool getConferenceByPrefix(const string& input, string& output)
	{
		
		LOG(INFO) << boost::str(boost::format("[prefix] %s") % input);
		
		JConfMap* jconfMap = JConfMap::getInstance();
		vector<JournalConferenceInfo*> confs;
		jconfMap->getConfWithPrefix(confs, input);

		mashaler::ConferenceScoredResult result;
		if(confs.size() && input.size()>1 && input[0]=='#')//get top conference
		{
			int count=0;
			if(input.size()>2)
			{
				count = atoi(input.c_str() + 2);
			}

			if(input[1]=='P')
			{
				sort(confs.begin(), confs.end(), ConfUtils::compare_with_pub_count);
			}
			else if(input[1]=='I')
			{
				sort(confs.begin(), confs.end(), ConfUtils::compare_with_score);
			}

			mashaler::ConfScorePair* pair;
			if(!count)//all
			{
				
				for (JournalConferenceInfo* info : confs)
				{
					pair = result.add_conf_score_pairs();
					pair->set_score(info->pub_ids.size());
					ProtoConverter::convert(*info, *(pair->mutable_conf()));
				}
			}
			else
			{
				for(int i=0;i<count && i<confs.size(); ++i)
				{
					pair = result.add_conf_score_pairs();
					pair->set_score(confs[i]->pub_ids.size());
					ProtoConverter::convert(*confs[i], *(pair->mutable_conf()));
				}
			}

			goto end;
		}
		
		if(confs.size())
		{
			sort(confs.begin(), confs.end(), ConfUtils::compare_with_name);
			mashaler::ConfScorePair* pair;
			for (JournalConferenceInfo* info : confs)
			{
				pair = result.add_conf_score_pairs();
				pair->set_score(info->pub_ids.size());
				ProtoConverter::convert(*info, *(pair->mutable_conf()));
			}
		}
end:
		return result.SerializeToString(&output);
	}
}
