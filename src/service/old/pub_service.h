#pragma once
#include "Logger.h"
#include<string>
using std::string;

namespace PubService
{

	bool searchPublications(const string& input, string& output);
	bool searchPublicationsWithScore(const string& input, string& output);//(string& query, int offset, int count, vector<string>& searchedFields, vector<string>& returnedFields);
	bool getPublicationsByNaId(const string& input, string& output);//(int naid, int offset, int count, vector<string>& returnedFields, vector<Condition>& conditions, string sortType);
	bool getPublicationsByConfId(const string& input, string& output);//(int confId, int offset, int count, vector<string>& returnedFields, vector<Condition>& conditions, string sortType);
	bool getPublicationsById(const string& input, string& output);//(vector<int>& pubId, vector<string>& returnedFields);
	bool getCitePublications(const string& input, string& output);
	bool getCitedByPulicataions(const string& input, string& output);
	bool addPublication(const string& input, string& output);
	bool updatePublication( const string& input, string& output);
	bool removePublications( const string& input, string& output);
	bool reloadPublications( const string& input, string& output);
	bool reviseAuthorIdNameMatches(const string& input, string& output);
	bool inferAuthorIdsOfPublication(const string& input, string& output);
	bool recommendRelatedPaper(const string& input, string& output);
};