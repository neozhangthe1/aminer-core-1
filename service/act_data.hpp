#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include "indexing/analyzer/ArnetAnalyzer.h"
#include "indexing/indexing.hpp"
#include "gflags/gflags.h"

#include "indexed_graph_cache.hpp"
#include "entity_searcher.hpp"

#define ACT_CACHE_UNINIT -2

using std::string;
using std::vector;
using std::pair;

struct ACTParam
{
	double alpha;
	double beta;
	int ntopics;
	int nauthors;
	int nconfs;
	int ndocs;
	int nwords;
	int nwordtoken;
	int liter;
};


class ACT
{
public:

	const double PRECISION;
	const int FILEBUFFERSIZE;
	const int STRINGBUFFERSIZE;

	ACT();
	~ACT();

	ACTParam parameter;

	void load();

	void addACTcount(int tid, int actxid, vector<unordered_map<int, int>>* actMap, int* sumMap);
	int getACTcount(int tid, int actxid, char xtype);

	int getAuthorNaIdByACTaid(int actaid);
	int getConfIdByACTcid(int actcid);
	int getPubIdByACTpid(int actpid);
	string getWordByACTwid(int actwid);

	int getACTaidByAuthorNaId(int naid);
	int getACTcidByConfid(int confid);
	int getACTpidByPubId(int pubid);
	int getACTwidByWord(string word);

	unordered_map<int, int>& getAmap(int tid);
	unordered_map<int, int>& getCmap(int tid);
	unordered_map<int, int>& getPmap(int tid);
	unordered_map<int, int>& getWmap(int tid);

public:
	int nTopic;
	int nWords;
	int nPublications;
	int nAuthors;
	int nConfs;

	vector<unordered_map<int, int>> _ACTa2countMap;
	vector<unordered_map<int, int>> _ACTc2countMap;
	vector<unordered_map<int, int>> _ACTp2countMap;
	vector<unordered_map<int, int>> _ACTw2countMap;

	int* _sumA2Count;
	int* _sumC2Count;
	int* _sumP2Count;
	int* _sumW2Count;

	unordered_map<int, string> _authormap;
	unordered_map<int, string> _confmap;
	unordered_map<int, int> _docmap;
	unordered_map<int, string> _wordmap;

	unordered_map<string, int> _reauthormap;
	unordered_map<string, int> _reconfmap;
	unordered_map<int, int> _redocmap;
	unordered_map<string, int> _rewordmap;

	void _loadTAssign(string path);
	void _loadOthers(string path);
	void _loadAuthorMap(string path);
	void _loadConfMap(string path);
	void _loadDocMap(string path);
	void _loadWordMap(string path);

	void _init();
private:

	//naid to aid in act model
	vector<int> act_aid_map;
	std::mutex act_aid_map_mutex;
	//conf_id to cid of act model
	vector<int> act_conf_map;
	std::mutex act_conf_map_mutex;
};

class ACTadapter
{
public:
	static ACTadapter* getInstance();

	ACTadapter();
	~ACTadapter();

	ACTParam& getParameter();
	vector<double> getTopicDistributionGivenAuthor(int naid); 
	vector<double> getTopicDistributionGivenAuthor(string name); 
	vector<double> getTopicDistributionGivenConf(int confid);
	vector<double> getTopicDistributionGivenConf(string confname);
	vector<double> getTopicDistributionGivenPub(int pubid, Publication const * pub = nullptr);
	vector<double> getTopicDistributionGivenQuery(string query); 
    
	unordered_map<int, double> getAuthorDistributionGivenTopic(int tid);
	unordered_map<int, double> getConfDistributionGivenTopic(int tid);
	unordered_map<int, double> getPubDistributionGivenTopic(int tid);
	unordered_map<string, double> getWordDistributionGivenTopic(int tid);

	// by Hang Su<su_hang@live.com> [July 23, 2011]
public:
	bool getTopTopicGivenAuthor(vector<pair<int, double>>& result, int naid);
	bool getTopTopicGivenPub(vector<pair<int, double>>& result, int pub_id);
	bool getTopTopicGivenConf(vector<pair<int, double>>& result, int conf_id);

	double getQueryProbabilityGivenAuthor(vector<string>& words, int naid);
	double getWordProbabilityGivenAuthor(string& word, int naid);
private:
	bool getTopTopic(vector<pair<int, double>>& result, vector<pair<int, double>>& distribution);
public:
	const int MAX_TOP_TOPIC_COUNT;
	const double TOPIC_THRESHOLD;
private:
	ACT _act;
};

namespace sae{namespace serialization{ namespace custom_serialization_impl{
    template<>
    struct serialize_impl<OSerializeStream, ACTParam>{
        static void run(OSerializeStream& ostr, const ACTParam& a){
            ostr << a.alpha << a.beta << a.ntopics << a.nauthors << a.nconfs << a.ndocs
				 << a.nwords << a.nwordtoken << a.liter;
        }
    };

    template <>
    struct deserialize_impl<ISerializeStream, ACTParam>{
        static void run(ISerializeStream& istr, ACTParam& a){
            istr >> a.alpha >> a.beta >> a.ntopics >> a.nauthors >> a.nconfs >> a.ndocs
				 >> a.nwords >> a.nwordtoken >> a.liter;
        }
    };

    template<>
    struct serialize_impl<OSerializeStream, ACT>{
        static void run(OSerializeStream& ostr, const ACT& a){
            ostr << a.parameter << a.nTopic << a.nWords << a.nPublications << a.nAuthors << a.nConfs
				 << a._ACTa2countMap << a._ACTc2countMap << a._ACTp2countMap << a._ACTw2countMap
				 << a._sumA2Count << a._sumC2Count << a._sumP2Count << a._sumW2Count
				 << a._authormap << a._confmap << a._docmap << a._wordmap
				 << a._reauthormap << a._reconfmap << a._redocmap << a._rewordmap;
        }
    };

    template <>
    struct deserialize_impl<ISerializeStream, ACT>{
        static void run(ISerializeStream& istr, ACT& a){
            istr >> a.parameter >> a.nTopic >> a.nWords >> a.nPublications >> a.nAuthors >> a.nConfs
				 >> a._ACTa2countMap >> a._ACTc2countMap >> a._ACTp2countMap >> a._ACTw2countMap
				 >> a._sumA2Count >> a._sumC2Count >> a._sumP2Count >> a._sumW2Count
				 >> a._authormap >> a._confmap >> a._docmap >> a._wordmap
				 >> a._reauthormap >> a._reconfmap >> a._redocmap >> a._rewordmap;
		}
    };
}}}
