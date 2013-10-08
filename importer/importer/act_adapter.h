#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "JConfMap.h"
#include "analyzer/ArnetAnalyzer.h"
#include "Logger.h"

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

	JConfMap* jConfMapInstance;

private:
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

	ACTParam& getParameter(); //返回当前model采用的参数，包括topic数、词表size、论文数量、作者数量、会议数量、迭代次数、alpha、beta、lowerbound
	vector<double> getTopicDistributionGivenAuthor(int naid); //返回P(topic_i|author)，可用于判断某作者属于哪个topic，找作者对应的topic
	vector<double> getTopicDistributionGivenAuthor(string name); //同上，输入改为作者名称
	vector<double> getTopicDistributionGivenConf(int confid); //返回P(topic_i|conf)，可用于判断某会议属于哪个topic，找会议对应的topic
	vector<double> getTopicDistributionGivenConf(string confname); //同上，输入改为会议名称
	vector<double> getTopicDistributionGivenPub(int pubid, Publication const * pub = nullptr); //返回P(topic_i|pub)，可用于判断某论文属于哪个topic，找论文对应的topic
	vector<double> getTopicDistributionGivenQuery(string query); //返回P(topic_i|query) = sigma_n(P(topic_i|word_j))/n，可用于判断某query属于哪个topic，找query对应的topic，query以非英文字母分隔，长时会慢，要先过滤非法字符再stem

	//这几个方法的数据量会很大，加排序？设lowerbound？
	unordered_map<int, double> getAuthorDistributionGivenTopic(int tid); //返回的Key是naid，Value是P(author_i|topic)，稀疏表示，可用于找某个topic最相关的作者
	unordered_map<int, double> getConfDistributionGivenTopic(int tid); //返回的Key是confid，Value是P(conf_i|topic)，稀疏表示，可用于找某个topic最相关的会议
	unordered_map<int, double> getPubDistributionGivenTopic(int tid); //返回的Key是pid，Value是P(pub_i|topic)，稀疏表示，可用于找某个topic最相关的论文，也可通过Map的size得知topic的范围大小
	unordered_map<string, double> getWordDistributionGivenTopic(int tid); //返回的Key是word，Value是P(word_i|topic)，稀疏表示，可用于找某个topic最相关的单词

	//其他新加的方法 by Hang Su<su_hang@live.com> [July 23, 2011]
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
