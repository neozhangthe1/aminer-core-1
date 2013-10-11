#include "act_service.hpp"
#include "act_data.hpp"
#include "interface.pb.h"

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
		
		//LOG(INFO) << boost::str(boost::format("[naid] %d") % naid);

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

		//LOG(INFO) << boost::str(boost::format("[athor_name] %s") % input);

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

		//LOG(INFO) << boost::str(boost::format("[conf_id] %s") % input);

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

		//LOG(INFO) << boost::str(boost::format("[athor_name] %s") % input);

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
		
		//LOG(INFO) << boost::str(boost::format("[pub] %s") % input);

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
		mashaler::EntityScoredResult result;
		mashaler::EntityScorePair* pair;
		
		//LOG(INFO) << boost::str(boost::format("[topic_author] %d") % params.ids(0));

		auto graph = IndexedGraphCache::instance().getGraph();
		auto vi = graph->g->Vertices();

		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<int, double>& probs = actadapter->getAuthorDistributionGivenTopic(params.ids(0));

		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			auto id = graph->idmap.find(std::make_pair("Author", iter->first))->second;
			vi->MoveTo(id);
			pair = result.add_entity_score_pairs();
			pair->set_score(iter->second);
			auto de = pair->mutable_entity();
			SearchServiceBase ssb(graph);
			ssb.fillEntity(de, vi.get());
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
		mashaler::EntityScoredResult result;
		mashaler::EntityScorePair* pair;
		
		//LOG(INFO) << boost::str(boost::format("[topic_pub] %d") % params.ids(0));

		auto graph = IndexedGraphCache::instance().getGraph();
		auto vi = graph->g->Vertices();

		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<int, double>& probs = actadapter->getPubDistributionGivenTopic(params.ids(0));

		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			auto id = graph->idmap.find(std::make_pair("Publication", iter->first))->second;
			vi->MoveTo(id);
			pair = result.add_entity_score_pairs();
			pair->set_score(iter->second);
			auto de = pair->mutable_entity();
			SearchServiceBase ssb(graph);
			ssb.fillEntity(de, vi.get());
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
		mashaler::EntityScoredResult result;
		mashaler::EntityScorePair* pair;
		
		//LOG(INFO) << boost::str(boost::format("[topic_conf] %d") % params.ids(0));

		auto graph = IndexedGraphCache::instance().getGraph();
		auto vi = graph->g->Vertices();

		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<int, double>& probs = actadapter->getConfDistributionGivenTopic(params.ids(0));
		JConf jconf;
		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			int key = iter->first;

			auto id = graph->idmap.find(std::make_pair("JConf", key))->second;
			vi->MoveTo(id);
			jconf = sae::serialization::convert_from_string<JConf>(vi->Data());
			if(&jconf)
			{
				pair = result.add_entity_score_pairs();
				pair->set_score(iter->second);
				auto de = pair->mutable_entity();
				SearchServiceBase ssb(graph);
				ssb.fillEntity(de, vi.get());
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
		
		//LOG(INFO) << boost::str(boost::format("[topic_word] %d") % input);

		ACTadapter* actadapter = ACTadapter::getInstance();
		const unordered_map<string, double>& probs = actadapter->getWordDistributionGivenTopic(tid);

		for (auto iter = probs.begin(); iter != probs.end(); iter ++)
		{
			pair = result.add_items();
			pair->set_key(iter->first);
			pair->set_value(iter->second);
		}
		return result.SerializeToString(&output);
	}
}

static void init(void *sender, void *args) {
    zrpc::RpcServer *server = reinterpret_cast<zrpc::RpcServer *>(args);
    LOG(INFO) << "loading act data.";
	ACTadapter* actadapter = ACTadapter::getInstance();
	server->addMethod(string("ACTService_getParameter"), &ACTService::getParameter);
	server->addMethod(string("ACTService_getTopicDistributionGivenAuthorNAid"), &ACTService::getTopicDistributionGivenAuthorNAid);
	server->addMethod(string("ACTService_getTopicDistributionGivenAuthorName"), &ACTService::getTopicDistributionGivenAuthorName);
	server->addMethod(string("ACTService_getTopicDistributionGivenConfId"), &ACTService::getTopicDistributionGivenConfId);
	server->addMethod(string("ACTService_getTopicDistributionGivenConfName"), &ACTService::getTopicDistributionGivenConfName);
	server->addMethod(string("ACTService_getTopicDistributionGivenPub"), &ACTService::getTopicDistributionGivenPub);
	server->addMethod(string("ACTService_getTopicDistributionGivenQuery"), &ACTService::getTopicDistributionGivenQuery);
	server->addMethod(string("ACTService_getAuthorDistributionGivenTopic"), &ACTService::getAuthorDistributionGivenTopic);
	server->addMethod(string("ACTService_getConfDistributionGivenTopic"), &ACTService::getConfDistributionGivenTopic);
	server->addMethod(string("ACTService_getPubDistributionGivenTopic"), &ACTService::getPubDistributionGivenTopic);
	server->addMethod(string("ACTService_getWordDistributionGivenTopic"), &ACTService::getWordDistributionGivenTopic);
    LOG(INFO) << "act service initialized.";
}

REGISTER_EVENT(init_actservice, init);