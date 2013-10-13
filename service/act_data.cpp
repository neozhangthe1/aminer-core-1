#include <string>
#include <stdlib.h>
#include <fstream>
#include <algorithm>

#include "act_service.hpp"
#include "act_data.hpp"
#include "entity_searcher.hpp"
#include "glog/logging.h"

DEFINE_string(actDir, "/var/saedata/actData/", "Directory of ACT data");
DECLARE_string(actdata);

ACT::ACT()
	:PRECISION(0.000001),FILEBUFFERSIZE(1024000),STRINGBUFFERSIZE(1024)
{
	nTopic = 200;
}

ACT::~ACT()//DONE
{
	delete _sumA2Count;
	delete _sumC2Count;
	delete _sumP2Count;
	delete _sumW2Count;
}

void ACT::load()//DONE
{
	LOG(INFO) << "Start loading act data..";
	string path = FLAGS_actDir;
	_loadOthers(path + "topic.others");
	nTopic = parameter.ntopics;
	if (nTopic == 0)
	{
		nTopic = 200;
	}
	_init();
	int arraySize = _ACTa2countMap.size();
	nWords = parameter.nwords;
	nPublications = parameter.ndocs;
	nAuthors = parameter.nauthors;
	nConfs = parameter.nconfs;
	for(int i=0; i<arraySize; i++)
	{
		_ACTa2countMap[i].rehash(nAuthors / nTopic * 30);
		_ACTp2countMap[i].rehash(nPublications / nTopic * 30);
		_ACTw2countMap[i].rehash(nWords / nTopic * 30);
		_ACTc2countMap[i].rehash(nConfs);
	}

	_sumA2Count = new int[nTopic + 1];
	memset(_sumA2Count, 0, sizeof(int)*(nTopic + 1));
	_sumC2Count = new int[nTopic + 1];
	memset(_sumC2Count, 0, sizeof(int)*(nTopic + 1));
	_sumP2Count = new int[nTopic + 1];
	memset(_sumP2Count, 0, sizeof(int)*(nTopic + 1));
	_sumW2Count = new int[nTopic + 1];
	memset(_sumW2Count, 0, sizeof(int)*(nTopic + 1));

	_loadTAssign(path + "topic.tassign");

	_loadAuthorMap(path + "authormap.txt");

	_loadConfMap(path + "confmap.txt");

	_loadDocMap(path + "docmap.txt");

	_loadWordMap(path + "wordmap.txt");

	LOG(INFO) << "Act data loaded..";
}

void ACT::addACTcount(int tid, int actxid, vector<unordered_map<int, int>>* actMap, int* sumMap )//DONE
{
	//Ïò_ACTa2countMap[tid]ÖÐ<actxid, count++>
	unordered_map<int, int>* amap1 = &(*actMap)[tid];
	unordered_map<int, int>::iterator intiter = amap1->find(actxid);
	if (intiter != amap1->end())
		intiter->second++;
	else
		(*amap1).insert(pair<int, int>(actxid, 1));

	//Ïò_ACTa2countMap[tid]ÖÐ<-1, count++>
	sumMap[tid] ++;

	//Ïò_ACTa2countMap[ntopic]ÖÐ<actxid, count++>
	unordered_map<int, int>* amap2 = &(*actMap)[nTopic];
	intiter = amap2->find(actxid);
	if (intiter != amap2->end())
		intiter->second++;
	else
		(*amap2)[actxid]= 1;

	//Ïò_ACTa2countMap[ntopic]ÖÐ<-1, count++>
	sumMap[nTopic] ++;
}

int ACT::getACTcount(int tid, int actxid, char xtype)//DONE
{
	int ntopic = parameter.ntopics;

	if (tid == -1)
		tid = ntopic;
	if (xtype == 'a')
	{
		if (actxid == -1)
			return _sumA2Count[tid];
		unordered_map<int, int>& amap = _ACTa2countMap[tid];
		unordered_map<int, int>::iterator iter = amap.find(actxid);
		if (iter!=amap.end())
			return iter->second;
		else
			return 0;
	}
	if (xtype == 'c')
	{
		if (actxid == -1)
			return _sumC2Count[tid];
		unordered_map<int, int>& cmap = _ACTc2countMap[tid];
		unordered_map<int, int>::iterator iter = cmap.find(actxid);
		if (iter!=cmap.end())
			return iter->second;
		else
			return 0;
	}
	if (xtype == 'p')
	{
		if (actxid == -1)
			return _sumP2Count[tid];
		unordered_map<int, int>& pmap = _ACTp2countMap[tid];
		unordered_map<int, int>::iterator iter = pmap.find(actxid);
		if (iter!=pmap.end())
			return iter->second;
		else
			return 0;
	}
	if (xtype == 'w')
	{
		if (actxid == -1)
			return _sumW2Count[tid];
		unordered_map<int, int>& wmap = _ACTw2countMap[tid];
		unordered_map<int, int>::iterator iter = wmap.find(actxid);
		if (iter!=wmap.end())
			return iter->second;
		else
			return 0;
	}
	return 0;
}

int ACT::getAuthorNaIdByACTaid(int actaid)//DONE
{
	unordered_map<int, string>::iterator iter = _authormap.find(actaid);
	string name = "";
	if (iter != _authormap.end())
		name = iter->second;

	auto gc = IndexedGraphCache::instance();
	auto searcher = EntitySearcher(gc.getGraph());
	auto sr = searcher.getAuthorByName(name);
	if (sr.size() > 0)
	{
		auto vi = gc.getGraph()->g->Vertices();
		vi->MoveTo(sr[0].docId);
		auto author = sae::serialization::convert_from_string<Author>(vi->Data());
		return author.id;
	}
	return -1;
}

int ACT::getConfIdByACTcid(int actcid)//DONE
{
	unordered_map<int, string>::iterator iter = _confmap.find(actcid);
	string confname = "";
	if (iter != _confmap.end())
		confname = iter->second;

	auto gc = IndexedGraphCache::instance();
	auto searcher = EntitySearcher(gc.getGraph());
	auto sr = searcher.getJConfByName(confname);
	if (sr.size() > 0)
	{
		auto vi = gc.getGraph()->g->Vertices();
		vi->MoveTo(sr[0].docId);
		auto jconf = sae::serialization::convert_from_string<JConf>(vi->Data());
		return jconf.id;
	}
	return -1;
}

int ACT::getPubIdByACTpid(int actpid)//DONE
{
	unordered_map<int, int>::iterator iter = _docmap.find(actpid);
	int pubid = -1;
	if (iter != _docmap.end())
		pubid = iter->second;
	return pubid;
}

string ACT::getWordByACTwid(int actwid)//DONE
{
	unordered_map<int, string>::iterator iter = _wordmap.find(actwid);
	string word = "";
	if (iter != _wordmap.end())
		word = iter->second;
	return word;
}

//Add a cache to speed up
int ACT::getACTaidByAuthorNaId(int naid)//DONE
{
	auto graph = IndexedGraphCache::instance().getGraph();
	if(act_aid_map.empty())
	{
		this->act_aid_map_mutex.lock();
		if(act_aid_map.empty())
		{
			act_aid_map.resize(graph->g->VertexCountOfType("Author"), ACT_CACHE_UNINIT);
		}
		this->act_aid_map_mutex.unlock();
	}
	//printf("aa");
	int size = act_aid_map.size();
	//printf("%d\n", size);
	if(naid >= size || naid<0)
		return -1;
	if(act_aid_map[naid]!=ACT_CACHE_UNINIT)
	{
		return act_aid_map[naid];
	}
	auto vi = graph->g->Vertices();
	auto id = graph->idmap.find(std::make_pair("Author", naid))->second;
	vi->MoveTo(id);
	auto author = sae::serialization::convert_from_string<Author>(vi->Data());
	unordered_map<string, int>::iterator iter;
	int actaid = -1;
	for (string& name: author.names)
	{
		iter = _reauthormap.find(name);
		if (iter != _reauthormap.end())
		{
			actaid = iter->second;
			break;
		}		
	}
	//boost::lock_guard<boost::mutex>(this->act_aid_map_mutex);
	act_aid_map[naid] = actaid;
	string name = author.names[0];
	
	return actaid;
}

//Add a cache to speed up
int ACT::getACTcidByConfid(int confid)//DONE
{
	auto graph = IndexedGraphCache::instance().getGraph();
	if(!act_conf_map.size())
	{
        std::lock_guard<std::mutex> lock(this->act_conf_map_mutex);
		if(!act_conf_map.size())
		{
			//in case of error
			act_conf_map.resize(graph->g->VertexCountOfType("JConf")+1000, ACT_CACHE_UNINIT);
		}
	}
	if(confid<0 || confid>=act_conf_map.size())
		return -1;
	if(act_conf_map[confid]!=ACT_CACHE_UNINIT)
	{
		return act_conf_map[confid];
	}

	auto vi = graph->g->Vertices();
	auto id = graph->idmap.find(std::make_pair("JConf", confid))->second;
	vi->MoveTo(id);
	auto jconf = sae::serialization::convert_from_string<JConf>(vi->Data());
	unordered_map<string, int>::iterator iter = _reauthormap.find(jconf.name);
	int actcid = -1;
	if (iter != _reconfmap.end())
		actcid = iter->second;
	//boost::lock_guard<boost::mutex>(this->act_conf_map_mutex);
	act_conf_map[confid] = actcid;
	return actcid;
}

int ACT::getACTpidByPubId(int pubid)//DONE
{
	unordered_map<int, int>::iterator iter = _redocmap.find(pubid);
	int actpid = -1;
	if (iter != _redocmap.end())
		actpid = iter->second;
	return actpid;
}

int ACT::getACTwidByWord(string word)//DONE
{
	//ACT do no need stemming
	unordered_map<string, int>::iterator iter = _rewordmap.find(word);
	int actwid = -1;
	if (iter != _rewordmap.end())
		actwid = iter->second;
	return actwid;
}

unordered_map<int, int>& ACT::getAmap(int tid)//DONE
{
	int ntopic = parameter.ntopics;
	if (tid == -1)
		tid = ntopic;
	return _ACTa2countMap[tid];
}

unordered_map<int, int>& ACT::getCmap(int tid)//DONE
{
	int ntopic = parameter.ntopics;
	if (tid == -1)
		tid = ntopic;
	return _ACTc2countMap[tid];
}

unordered_map<int, int>& ACT::getPmap(int tid)//DONE
{
	int ntopic = parameter.ntopics;
	if (tid == -1)
		tid = ntopic;
	return _ACTp2countMap[tid];
}

unordered_map<int, int>& ACT::getWmap(int tid)//DONE
{
	int ntopic = parameter.ntopics;
	if (tid == -1)
		tid = ntopic;
	return _ACTw2countMap[tid];
}

void ACT::_init()//DONE
{
	unordered_map<int, int> tempmap;
	for (int i = 0; i <= nTopic; i ++)
	{
		_ACTa2countMap.push_back(tempmap);
		_ACTc2countMap.push_back(tempmap);
		_ACTp2countMap.push_back(tempmap);
		_ACTw2countMap.push_back(tempmap);
	}
}

void ACT::_loadTAssign(string path)//DONE
{
	char* lineBuffer;
	FILE* file = fopen(path.c_str(), "r");
	int pid = 0;
	int cid = 0;
	int mod3Index = 0;
	int lineOffset = 0;
	int num = 0;
	int aid = 0;
	int tid = 0;
	int wid = 0;
	int splidx = 0;

	lineBuffer = new char[FILEBUFFERSIZE];
	while(!feof(file))
	{
		int charCount = fread(lineBuffer, sizeof(char), FILEBUFFERSIZE, file);
		char* currentPos = lineBuffer;

		while(charCount)
		{
			if(*currentPos == '\n')
			{
				lineOffset = -1;
				mod3Index++;

				if(mod3Index%3==1)
				{
					cid = 0;
				}
				else if(mod3Index%3 == 2)
				{
					aid = 0;
					tid = 0;
					wid = 0;
					num = 0;
					splidx = 0;
					pid = mod3Index / 3;
				}
				/*if (mod3Index % 10000 == 0)
					cout<<mod3Index<<endl;*/
			}
			else if(mod3Index%3==1)
			{
				// skip first two charactors: #c
				if(lineOffset >= 2)
				{
					cid = cid * 10 + *currentPos - '0';
				}
			}
			else if (mod3Index%3==2)
			{
				char c = *(currentPos);
				if (c >= '0' && c <= '9')
				{
					num = num * 10 + c - '0';
				}
				else if (c == ':')
				{
					if (splidx == 0)
						wid = num;
					if (splidx == 1)
						tid = num;
					splidx ++;
					num = 0;
				}
				else if (c == ' ' && splidx == 2)
				{
					aid = num;
					num = 0;
					splidx = 0;
					addACTcount(tid, aid, &_ACTa2countMap, _sumA2Count);
					addACTcount(tid, cid, &_ACTc2countMap, _sumC2Count);
					addACTcount(tid, pid, &_ACTp2countMap, _sumP2Count);
					addACTcount(tid, wid, &_ACTw2countMap, _sumW2Count);
					aid = 0;
					tid = 0;
					wid = 0;
				}
			}
			charCount--;
			lineOffset++;
			++currentPos;
		}
	}
	delete[] lineBuffer;
	fclose(file);
}

void ACT::_loadOthers(string path)
{
	FILE* file = fopen(path.c_str(), "r");
	char* str;
	double score;
	char* pPos;
	string name;

	if(file == NULL)
	{
		printf("cannot find file %s.\n", path.c_str());
	}
	else
	{
		str = new char[STRINGBUFFERSIZE];
		while(!feof(file) && fscanf(file, "%s", str) > 0)
		{
			pPos = str;

			while(*pPos != '=')
			{
				++pPos;
			}
			*pPos = '\0';
			++pPos;
			score = atof(pPos);
			name = string(str);

			if(name=="alpha")
			{
				parameter.alpha = score;
			}
			else if(name=="beta")
			{
				parameter.beta = score;
			}
			else if(name=="ntopics")
			{
				parameter.ntopics = score;
			}
			else if(name=="nauthors")
			{
				parameter.nauthors = score;
			}
			else if(name=="nconfs")
			{
				parameter.nconfs = score;
			}
			else if(name=="ndocs")
			{
				parameter.ndocs = score;
			}
			else if(name=="nwords")
			{
				parameter.nwords = score;
			}
			else if(name=="nwordtoken")
			{
				parameter.nwordtoken = score;
			}
			else if(name=="liter")
			{
				parameter.liter = score;
			}
		}
		delete[] str;
		fclose(file);
	}
}

void ACT::_loadAuthorMap(string path)//DONE
{
	char* lineBuffer;
	char* str;
	FILE* file = fopen(path.c_str(), "r");
	int lineOffset = 0;
	int aid = 0;
	int size;
	bool isName = true;

	str = new char[STRINGBUFFERSIZE];

	lineBuffer = (char*)malloc(FILEBUFFERSIZE);
	fscanf(file, "%d\n", &size);
	_authormap.rehash(size);
	_reauthormap.rehash(size);
	while(!feof(file))
	{
		int charCount = fread(lineBuffer, sizeof(char), FILEBUFFERSIZE, file);
		char* currentPos = lineBuffer;

		while(charCount)
		{
			if(*currentPos == '\n')
			{
				lineOffset = -1;
				_authormap.insert(pair<int, string>(aid, str));
				_reauthormap.insert(pair<string, int>(str, aid));
				aid = 0;
				isName = true;
			}
			else if(*currentPos == '\t')
			{
				str[lineOffset] = '\0';
				isName = false;
			}
			else
			{
				if(isName)
				{
					str[lineOffset] = *(currentPos);
				}
				else
				{
					aid = aid * 10 + *currentPos - '0';
				}
			}
			--charCount;
			++lineOffset;
			++currentPos;
		}
	}
	delete[] str;
	free(lineBuffer);
	fclose(file);
}

void ACT::_loadConfMap(string path)//DONE
{
	char* lineBuffer;
	char* str;
	FILE* file = fopen(path.c_str(), "r");
	int lineOffset = 0;
	int cid = 0;
	int size;
	bool isName = true;
	string confName;

	str = new char[STRINGBUFFERSIZE];

	lineBuffer = (char*)malloc(FILEBUFFERSIZE);
	fscanf(file, "%d\n", &size);
	_confmap.rehash(size);
	_reconfmap.rehash(size);
	while(!feof(file))
	{
		int charCount = fread(lineBuffer, sizeof(char), FILEBUFFERSIZE, file);
		char* currentPos = lineBuffer;

		while(charCount)
		{
			if(*currentPos == '\n')
			{
				lineOffset = -1;
				confName = string(str);
				_confmap.insert(pair<int, string>(cid, confName));
				_reconfmap.insert(pair<string, int>(confName, cid));
				cid = 0;
				isName = true;
			}
			else if(*currentPos == '\t')
			{
				str[lineOffset] = '\0';
				isName = false;
			}
			else
			{
				if(isName)
				{
					str[lineOffset] = *(currentPos);
				}
				else
				{
					cid = cid * 10 + *currentPos - '0';
				}
			}
			--charCount;
			++lineOffset;
			++currentPos;
		}
	}
	delete[] str;
	free(lineBuffer);
	fclose(file);
}

void ACT::_loadDocMap(string path)//DONE
{
	FILE* file = fopen(path.c_str(), "r");
	int size = 0;
	fscanf(file, "%d\n", &size);
	_docmap.rehash(size);
	_redocmap.rehash(size);
	for (int i = 0; i < size; i ++)
	{
		int pid;
		fscanf(file, "%d\n", &pid);
		_docmap[i] = pid;
		_redocmap[pid] = i;
	}
	fclose(file);
}

void ACT::_loadWordMap(string path)//DONE
{
	FILE* file = fopen(path.c_str(), "r");
	int size = 0;
	char* str;
	string word;

	str = new char[STRINGBUFFERSIZE];
	fscanf(file, "%d\n", &size);
	_wordmap.rehash(size);
	_rewordmap.rehash(size);
	for (int i = 0; i < size; i ++)
	{
		int wid;
		fscanf(file, "%s %d\n", str, &wid);
		word = string(str);
		_wordmap.insert(pair<int, string>(wid,  word));
		_rewordmap.insert(pair<string, int>(word,  wid));
	}
	delete[] str;
	fclose(file);
}

ACTadapter* ACTadapter::getInstance()
{
	static ACTadapter instance;
	return &instance;
}

ACTParam& ACTadapter::getParameter()//DONE
{
	return _act.parameter;
}

vector<double> ACTadapter::getTopicDistributionGivenAuthor(int naid)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;

	vector<double> result(ntopic, 1/(double)ntopic);
	/*for (int tid = 0; tid < ntopic; tid ++)
	result[tid] = 0.0;*/

	int aid = _act.getACTaidByAuthorNaId(naid);
	if(aid==-1)
		goto end;
	for (int tid = 0; tid < ntopic; tid ++)
	{
		double dividend = _act.getACTcount(tid, aid, 'a') + alpha;
		double divisor = _act.getACTcount(-1, aid, 'a') + alpha * ntopic;
		if (abs(dividend) < _act.PRECISION) 
			result[tid] = 0;
		else
			result[tid] = dividend / divisor;
	}
end:
	return result;
}

vector<double> ACTadapter::getTopicDistributionGivenConf(int confid)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;

	vector<double> result(ntopic, 0.0);
	/*for (int tid = 0; tid < ntopic; tid ++)
	result[tid] = 0.0;*/

	int cid = _act.getACTcidByConfid(confid);
	for (int tid = 0; tid < ntopic; tid ++)
	{
		double dividend = _act.getACTcount(tid, cid, 'c') + alpha;
		double divisor = _act.getACTcount(-1, cid, 'c') + alpha * ntopic;
		if (abs(dividend) < _act.PRECISION)
			result[tid] = 0;
		else
			result[tid] = dividend / divisor;
	}
	return result;
}

vector<double> ACTadapter::getTopicDistributionGivenPub(int pubid, Publication const * pub)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;

	vector<double> result(ntopic, 0.0);
	auto graph = IndexedGraphCache::instance().getGraph();

	int pid = _act.getACTpidByPubId(pubid);
	if (pid == -1)
	{
        if (!pub) {
    		auto vi = graph->g->Vertices();
			auto id = graph->idmap.find(std::make_pair("Publication", pubid))->second;
			vi->MoveTo(id);
			Publication p = sae::serialization::convert_from_string<Publication>(vi->Data());
			pub = &p;
        }
		if (pub)
			return getTopicDistributionGivenQuery(pub->title);
		return result;
	}
	
	for (int tid = 0; tid < ntopic; tid ++)
	{
		double dividend = _act.getACTcount(tid, pid, 'p') + alpha;
		double divisor = _act.getACTcount(-1, pid, 'p') + alpha * ntopic;
		if (abs(dividend) < _act.PRECISION)
			result[tid] = 0;
		else
			result[tid] = dividend / divisor;
	}
	return result;
}

vector<double> ACTadapter::getTopicDistributionGivenQuery(string query)//DONE
{
	TokenStream* tokenStream = ArnetAnalyzer::tokenStream(query);
	Token token;
	vector<string> words;
	//boost::split(words, query, boost::is_any_of(" ,\t\r\n"), boost::token_compress_on);
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;
	double beta = _act.parameter.beta;
	int nwords = _act.parameter.nwords;
	vector<double> result(ntopic, 0.0);
	/*for (int tid = 0; tid < ntopic; tid ++)
	result[tid] = 0.0;*/

	for (int tid = 0; tid < ntopic; tid ++)
	{
		for (int i = 0; i < words.size(); i++ )
		{
			string word = words[i];
			int wid = _act.getACTwidByWord(word);
			double dividend = (_act.getACTcount(tid, wid, 'w') + beta) * _act.getACTcount(tid, -1, 'w') * (_act.getACTcount(-1, -1, 'w') + beta * nwords);
			double divisor = (_act.getACTcount(tid, -1, 'w') + beta) * _act.getACTcount(-1, -1, 'w') * (_act.getACTcount(-1, wid, 'w') + beta) * nwords;
			if (abs(dividend) < _act.PRECISION)
				result[tid] += 0;
			else 
				result[tid] += dividend / divisor;
		}
	}
	return result;
}

unordered_map<int, double> ACTadapter::getAuthorDistributionGivenTopic(int tid)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;
	int nauthor = _act.parameter.nauthors;
	int nwords = _act.parameter.nwords;

	unordered_map<int, double> result;
	unordered_map<int, int>& amap = _act.getAmap(tid);
	//unordered_map<int, int>::iterator iter;
	for (unordered_map<int, int>::iterator iter = amap.begin(); iter!=amap.end(); iter++ )
	{
		int aid = iter->first;
		int naid = _act.getAuthorNaIdByACTaid(aid);
		if(naid != -1)
		{
			double dividend = _act.getACTcount(tid, aid, 'a') + alpha;
			double divisor = _act.getACTcount(tid, -1, 'a') + alpha	* nauthor;
			if (abs(dividend) < _act.PRECISION) 
				result[naid] = 0;
			else 
				result[naid] = dividend / divisor;
		}
	}
	return result;
}

unordered_map<int, double> ACTadapter::getConfDistributionGivenTopic(int tid)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;
	int nconf = _act.parameter.nconfs;
	int nwords = _act.parameter.nwords;

	unordered_map<int, double> result;
	unordered_map<int, int>& cmap = _act.getCmap(tid);
	for (unordered_map<int, int>::iterator iter = cmap.begin(); iter!= cmap.end(); iter ++ )
	{
		int cid = iter->first;
		int confid = _act.getConfIdByACTcid(cid);
		if(confid!=-1)
		{

			double dividend = _act.getACTcount(tid, cid, 'c') + alpha;
			double divisor = _act.getACTcount(tid, -1, 'c') + alpha	* nconf;
			if (abs(dividend) < _act.PRECISION) 
				result[confid] = 0;
			else 
				result[confid] = dividend / divisor;
		}
	}
	return result;
}

unordered_map<int, double> ACTadapter::getPubDistributionGivenTopic(int tid)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;
	int npub = _act.parameter.ndocs;

	unordered_map<int, double> result;
	unordered_map<int, int>& pmap = _act.getPmap(tid);
	//unordered_map<int, int>::iterator iter;
	for(unordered_map<int, int>::iterator iter = pmap.begin(); iter!=pmap.end(); iter++ )
	{
		int pid = iter->first;
		int pubid = _act.getPubIdByACTpid(pid);
		if(pubid!=-1)
		{
			double dividend = _act.getACTcount(tid, pid, 'p') + alpha;
			double divisor = _act.getACTcount(tid, -1, 'p') + alpha	* npub;
			if (abs(dividend) < _act.PRECISION) 
				result[pubid] = 0;
			else 
				result[pubid] = dividend / divisor;
		}
	}
	return result;
}

unordered_map<string, double> ACTadapter::getWordDistributionGivenTopic(int tid)//DONE
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;
	double beta = _act.parameter.beta;
	int nwords = _act.parameter.nwords;

	unordered_map<string, double> result;
	unordered_map<int, int>& wmap = _act.getWmap(tid);
	//unordered_map<int, int>::iterator iter;
	for (unordered_map<int, int>::iterator iter = wmap.begin(); iter!= wmap.end(); iter ++)
	{
		int wid = iter->first;
		string word = _act.getWordByACTwid(wid);
		double dividend = _act.getACTcount(tid, wid, 'w') + beta;
		double divisor = _act.getACTcount(tid, -1, 'w') + beta * nwords;
		if (abs(dividend) < _act.PRECISION) 
			result[word] = 0;
		else 
			result[word] = dividend / divisor;
	}
	return result;
}

vector<double> ACTadapter::getTopicDistributionGivenAuthor(string name)//DONE
{
	auto gc = IndexedGraphCache::instance();
	auto searcher = EntitySearcher(gc.getGraph());
	auto sr = searcher.getAuthorByName(name);
	int naid = -2;

	if (sr.size() > 0)
	{
		auto vi = gc.getGraph()->g->Vertices();
		vi->MoveTo(sr[0].docId);
		auto author = sae::serialization::convert_from_string<Author>(vi->Data());
		naid = author.id;
	}

	return getTopicDistributionGivenAuthor(naid);
}

vector<double> ACTadapter::getTopicDistributionGivenConf(string confname)//DONE
{
	auto gc = IndexedGraphCache::instance();
	auto searcher = EntitySearcher(gc.getGraph());
	auto sr = searcher.getJConfByName(confname);
	if (sr.size() > 0)
	{
		auto vi = gc.getGraph()->g->Vertices();
		vi->MoveTo(sr[0].docId);
		auto jconf = sae::serialization::convert_from_string<JConf>(vi->Data());
		int confid = jconf.id;
		return getTopicDistributionGivenConf(confid);
	}
	return std::vector<double>{};
}

ACTadapter::ACTadapter()//DONE
	:MAX_TOP_TOPIC_COUNT(5),
	TOPIC_THRESHOLD(0.05)
{
    struct stat buf;
    int result;
    result =stat(FLAGS_actdata.c_str(), &buf );

    if (result != 0) {
        LOG(INFO) << "Serialized actdata file not found";
        _act.load();
        std::ofstream fout("aminer.act", std::fstream::binary);
        sae::serialization::OSerializeStream encoder(&fout);
        encoder << _act;
        fout.close();
    }
    LOG(INFO) << "Load serialized act data..";
    std::ifstream finv(FLAGS_actdata.c_str());
    sae::serialization::ISerializeStream decoderv(&finv);
    LOG(INFO) << "Deserializing...";
    decoderv >> _act;
    finv.close();
    LOG(INFO) << "Serialized actdata loaded..";
}

ACTadapter::~ACTadapter()//DONE
{
}

bool topic_compare(pair<int, double> left, pair<int, double> right)
{
	return left.second>right.second;
}

bool ACTadapter::getTopTopicGivenAuthor(vector<pair<int, double>>& result, int naid)
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;

	vector<pair<int,double>> distribution;
	distribution.reserve(ntopic);

	int aid = _act.getACTaidByAuthorNaId(naid);
	if(aid==-1)
	{
		return false;
	}
	else
	{
		double divisor = _act.getACTcount(-1, aid, 'a') + alpha * ntopic;
		for (int tid = 0; tid < ntopic; tid ++)
		{
			double dividend = _act.getACTcount(tid, aid, 'a') + alpha;
			if (abs(dividend) >= _act.PRECISION)
			{
				distribution.push_back(std::make_pair(tid, dividend / divisor));
			}
		}
	}	
	return getTopTopic(result, distribution);
}

bool ACTadapter::getTopTopicGivenPub(vector<pair<int, double>>& result, int pub_id)
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;

	vector<pair<int,double>> distribution;
	distribution.reserve(ntopic);

	int pid = _act.getACTpidByPubId(pub_id);
	if(pid==-1)
	{
		auto graph = IndexedGraphCache::instance().getGraph();
		auto vi = graph->g->Vertices();
		auto id = graph->idmap.find(std::make_pair("Publication", pub_id))->second;
		vi->MoveTo(id);
		auto pub = sae::serialization::convert_from_string<Publication>(vi->Data());
		vector<double> temp=getTopicDistributionGivenQuery(pub.title);
		for(int i=0;i<ntopic;++i)
		{
			distribution.push_back(std::make_pair(i, temp[i]));
		}	
	}
	else
	{
		double divisor = _act.getACTcount(-1, pid, 'p') + alpha * ntopic;
		for (int tid = 0; tid < ntopic; tid ++)
		{
			double dividend = _act.getACTcount(tid, pid, 'p') + alpha;
			if (abs(dividend) >= _act.PRECISION)
			{
				distribution.push_back(std::make_pair(tid, dividend / divisor));
			}
		}
	}
	
	return getTopTopic(result, distribution);
}

bool ACTadapter::getTopTopicGivenConf(vector<pair<int, double>>& result, int conf_id)
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;

	vector<pair<int,double>> distribution;
	distribution.reserve(ntopic);

	int cid = _act.getACTcidByConfid(conf_id);
	if(cid==-1)
	{
		return false;
	}
	else
	{
		double divisor = _act.getACTcount(-1, cid, 'c') + alpha * ntopic;
		for (int tid = 0; tid < ntopic; tid ++)
		{
			double dividend = _act.getACTcount(tid, cid, 'c') + alpha;
			if (abs(dividend) >= _act.PRECISION)
			{
				distribution.push_back(std::make_pair(tid, dividend / divisor));
			}
		}
	}	
	return getTopTopic(result, distribution);
}

bool ACTadapter::getTopTopic(vector<pair<int, double>>& result, vector<pair<int, double>>& distribution)
{
	if(distribution.size()>this->MAX_TOP_TOPIC_COUNT)
	{
		nth_element(distribution.begin(), distribution.begin()+MAX_TOP_TOPIC_COUNT, distribution.end(), topic_compare);
	}
	
	double sum = 0.0;
	int size = 0;
	for(int i=0;i<MAX_TOP_TOPIC_COUNT && i<distribution.size();++i)
	{
		if(distribution[i].second>TOPIC_THRESHOLD)
		{
			sum+=distribution[i].second;
			++size;
		}
	}
	result.reserve(size);
	for(int i=0;i<MAX_TOP_TOPIC_COUNT && i<distribution.size();++i)
	{
		if(distribution[i].second>TOPIC_THRESHOLD)
		{
			result.push_back(std::make_pair(distribution[i].first, distribution[i].second/sum));
		}
	}
	return true;
}

double ACTadapter::getWordProbabilityGivenAuthor(string& word, int naid)
{
	int ntopic = _act.parameter.ntopics;
	double alpha = _act.parameter.alpha;
	double beta = _act.parameter.beta;
	int nwords = _act.parameter.nwords;

	double result = 0.0;
	vector<double> distribution = getTopicDistributionGivenAuthor(naid);
	int wid = _act.getACTwidByWord(word);

	for(int tid=0;tid<ntopic;++tid)
	{
		double dividend = _act.getACTcount(tid, wid, 'w') + beta;
		double divisor = _act.getACTcount(tid, -1, 'w') + beta * nwords;
		result += distribution[tid]*dividend/divisor;
	}
	return result;
}

double ACTadapter::getQueryProbabilityGivenAuthor(vector<string>& words, int naid)
{
	double p = 1.0;
	for (string& word:words)
	{
		p *= getWordProbabilityGivenAuthor(word, naid);
	}
	return p;
}
