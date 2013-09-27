#include "act_service.h"
#include "adapter/act_adapter.h"
#include "IndexResource.h"
#include "interface.pb.h"
#include "InterfaceUtils.h"
#include "Logger.h"

/*
*Wrapper of ACTadapter
*Author : Haoquan
*
*Update by Hang Su<su_hang@live.com> 
*Date: June 20, 2011
*/
namespace ACTService
{
	/*
	*Input: empty string
	*Output: mashaler::Distribution
	*/
	bool getParameter(const string& input, string& output)
	{
		mashaler::StringDoublePairs result;
		mashaler::StringDoublePair* temp;

		LOG(INFO) << "[param]";

		ACTadapter* actadapter = ACTadapter::getInstance();
		ACTParam& param = actadapter->getParameter();
	
		temp = result.add_items();
		temp->set_key("alpha");
		temp->set_value(param.alpha);
		
		temp = result.add_items();
		temp->set_key("beta");
		temp->set_value(param.beta);

		temp = result.add_items();
		temp->set_key("ntopics");
		temp->set_value(param.ntopics);

		temp = result.add_items();
		temp->set_key("nauthors");
		temp->set_value(param.nauthors);

		temp = result.add_items();
		temp->set_key("nconfs");
		temp->set_value(param.nconfs);

		temp = result.add_items();
		temp->set_key("ndocs");
		temp->set_value(param.ndocs);

		temp = result.add_items();
		temp->set_key("nwords");
		temp->set_value(param.nwords);

		temp = result.add_items();
		temp->set_key("nwordtoken");
		temp->set_value(param.nwordtoken);

		temp = result.add_items();
		temp->set_key("liter");
		temp->set_value(param.liter);
		return result.SerializeToString(&output);
	}


	bool getTopicDistributionGivenAuthorNAid(const string& input, string& output)//(int naid)
	{
		int naid = atoi(input.c_str());
		
		LOG(INFO) << boost::str(boost::format("[naid] %d") % naid);

		mashaler::Distribution result;

		ACTadapter* actadapter = ACTadapter::getInstance();
		const vector<double>& probs = actadapter->getTopicDistributionGivenAuthor(naid);

		for (int tid = 0; tid < probs.size(); tid ++)
		{
			result.add_values(probs[tid]);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: string
	*Output: mashaler::Distribution
	*/
	bool getTopicDistributionGivenAuthorName(const string& input, string& output)//(string name)
	{
		mashaler::Distribution result;

		LOG(INFO) << boost::str(boost::format("[athor_name] %s") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const vector<double>& probs = actadapter->getTopicDistributionGivenAuthor(input);
		for (int tid = 0; tid < probs.size(); tid ++)
		{
			result.add_values(probs[tid]);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: confid toString
	*Output: mashaler::Distribution
	*/
	bool getTopicDistributionGivenConfId(const string& input, string& output)//(int confid)
	{
		int confid = atoi(input.c_str());
		mashaler::Distribution result;

		LOG(INFO) << boost::str(boost::format("[conf_id] %s") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const vector<double>& probs = actadapter->getTopicDistributionGivenConf(confid);
		for (int tid = 0; tid < probs.size(); tid ++)
		{
			result.add_values(probs[tid]);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: string
	*Output: mashaler::Distribution
	*/
	bool getTopicDistributionGivenConfName(const string& input, string& output)//(string confname)
	{
		mashaler::Distribution result;

		LOG(INFO) << boost::str(boost::format("[athor_name] %s") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const vector<double>& probs = actadapter->getTopicDistributionGivenConf(input);
		for (int tid = 0; tid < probs.size(); tid ++)
		{
			result.add_values(probs[tid]);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: pubid toString
	*Output: mashaler::Distribution
	*/
	bool getTopicDistributionGivenPub(const string& input, string& output)//(string confname)
	{
		int pubid = atoi(input.c_str());
		mashaler::Distribution result;
		
		LOG(INFO) << boost::str(boost::format("[pub] %s") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const vector<double>& probs = actadapter->getTopicDistributionGivenPub(pubid);
		for (int tid = 0; tid < probs.size(); tid ++)
		{
			result.add_values(probs[tid]);
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: string
	*Output: mashaler::Distribution
	*/
	bool getTopicDistributionGivenQuery(const string& input, string& output)//(string confname)
	{
		mashaler::Distribution result;
		
		LOG(INFO) << boost::str(boost::format("[query] %s") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const vector<double>& probs = actadapter->getTopicDistributionGivenQuery(input);
		for (int tid = 0; tid < probs.size(); tid ++)
		{
			result.add_values(probs[tid]);
		}
		return result.SerializeToString(&output);
	}
	/*
	*Input: mashaler::IntQueryParams
	*Output: mashaler::AuthorScoredResult
	*/
	bool getAuthorDistributionGivenTopic(const string& input, string& output)//(int tid, vector<string> returnedFields)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		if(params.ids_size()==0)
			return false;
		mashaler::AuthorScoredResult result;
		mashaler::AuthorScorePair* pair;
		
		LOG(INFO) << boost::str(boost::format("[topic_author] %d") % params.ids(0));

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* acollection = resource.authorCollection();
		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<int, double>& probs = actadapter->getAuthorDistributionGivenTopic(params.ids(0));
		Document* doc;

		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			doc = acollection->getDocumentByIndex(iter->first);
			if(doc==NULL)
				continue;
			pair = result.add_author_score_pairs();
			pair->set_score(iter->second);
			ProtoConverter::convertAuthor(acollection, *(Author*)doc, *(pair->mutable_author()), params.returned_fields());
		}
		return result.SerializeToString(&output);
	}
	/*
	*Input: mashaler::IntQueryParams
	*Output: mashaler::PublicationScoredResult
	*/
	bool getPubDistributionGivenTopic(const string& input, string& output)//(int tid, vector<string> returnedFields)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		if(params.ids_size()==0)
			return false;
		mashaler::PublicationScoredResult result;
		mashaler::PubScorePair* pair;
		
		LOG(INFO) << boost::str(boost::format("[topic_pub] %d") % params.ids(0));

		IndexResource& resource = IndexResource::instance();
		DocumentCollection* collection = resource.pubCollection();
		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<int, double>& probs = actadapter->getPubDistributionGivenTopic(params.ids(0));
		Document* doc;

		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			doc = collection->getDocumentByIndex(iter->first);
			if(doc==NULL)
				continue;
			pair = result.add_pub_score_pairs();
			pair->set_score(iter->second);
			ProtoConverter::convertPub(collection, *doc, *(pair->mutable_publication()), params.returned_fields());
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: mashaler::IntQueryParams
	*Output: mashaler::ConferenceScoredResult
	*/
	bool getConfDistributionGivenTopic(const string& input, string& output)//(int tid)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		if(params.ids_size()==0)
			return false;
		mashaler::ConferenceScoredResult result;
		mashaler::ConfScorePair* pair;
		
		LOG(INFO) << boost::str(boost::format("[topic_conf] %d") % params.ids(0));

		JConfMap* confadapter = JConfMap::getInstance();
		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<int, double>& probs = actadapter->getConfDistributionGivenTopic(params.ids(0));
		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			int key = iter->first;
			double value = iter->second;

			JournalConferenceInfo* jconf = confadapter->getJConf(key);
			assert(jconf!=NULL);
			if(jconf)
			{
				pair = result.add_conf_score_pairs();
				pair->set_score(iter->second);
				ProtoConverter::convert(*jconf, *pair->mutable_conf());
			}
		}
		return result.SerializeToString(&output);
	}

	/*
	*Input: topic id toString
	*Output: mashaler::StringDoublePairs
	*/
	bool getWordDistributionGivenTopic(const string& input, string& output)//(int tid)
	{
		int tid = atoi(input.c_str());
		mashaler::StringDoublePairs result;
		mashaler::StringDoublePair* pair;
		
		LOG(INFO) << boost::str(boost::format("[topic_word] %d") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<string, double>& probs  = actadapter->getWordDistributionGivenTopic(tid);

		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			pair = result.add_items();
			pair->set_key(iter->first);
			pair->set_value(iter->second);
		}
		return result.SerializeToString(&output);
	}
}






