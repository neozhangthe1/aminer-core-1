#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Logger.h"
using std::vector;
using std::string;

class Publication;

class JournalConferenceInfo
{
public:
	/* Primary Key */
	int ID;
	/* Journal or Conference Name */
	string name;
	/* Journal or Conference Score */
	double score;
	/* index */
	int index;
	/* percent */
	double percent;
	/* alias */
	string alias;

	vector<int> pub_ids;
	//default constructor
	JournalConferenceInfo();
	JournalConferenceInfo(int _ID, string& _name, double _score, int _index, double _percent, string& _alias);
};


class JConfMap
{
public:

	~JConfMap();
	static JConfMap* getInstance();

	string getName(int id);

	string getAlias(int id);

	double getScore(int id);

	double getScoreByName(const string& name);

	int getIdByName(string name);

	bool containsConf(string name);

	JournalConferenceInfo* getJConf(int id);

	int getSize();

	double getImpact(int id);

	bool getConfWithPrefix(vector<JournalConferenceInfo*>& results, string prefix);
private:
	JConfMap();

	void destroy();

	void load();

	void loadImpact();
private:
	const static string tableName;
	const static string sql;

    std::unordered_map<int, JournalConferenceInfo*> jconfhash;
	std::unordered_map<string, JournalConferenceInfo*> jconfnamehash;
	std::unordered_map<int, double> impact_factor;


};
