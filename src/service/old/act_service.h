#pragma once
#include "Logger.h"
#include<string>
using std::string;

namespace ACTService
{

	bool getParameter(const string& input, string& output);//();
	bool getTopicDistributionGivenAuthorNAid(const string& input, string& output);//(int naid);
	bool getTopicDistributionGivenAuthorName(const string& input, string& output);//(string name);
	bool getTopicDistributionGivenConfId(const string& input, string& output);//(int confid);
	bool getTopicDistributionGivenConfName(const string& input, string& output);//(string confname);
	bool getTopicDistributionGivenPub(const string& input, string& output);//(int pubid);
	bool getTopicDistributionGivenQuery(const string& input, string& output);//(string query);

	bool getAuthorDistributionGivenTopic(const string& input, string& output);//(int tid, vector<string> returnedFields);
	bool getConfDistributionGivenTopic(const string& input, string& output);//(int tid);
	bool getPubDistributionGivenTopic(const string& input, string& output);//(int tid, vector<string> returnedFields);
	bool getWordDistributionGivenTopic(const string& input, string& output);//(int tid);

};