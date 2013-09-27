#pragma once
#include "Logger.h"
#include<string>
using std::string;

#define RERANK_SIZE 200

class Author;

namespace AuthorService
{

	bool searchAuthors(const string& input, string& output);//(string& query, int offset, int count, vector<string>& returnedFields);
	bool getAuthorsById(const string& input, string& output);//(vector<int>& naIds, vector<string>& returnedFields);
	bool getAuthorsByName(const string& input, string& output);//(string& name, vector<string>& returnedFields);
	bool getAuthorsWithSameNameById(const string& input, string& output);//
	bool getAuthorsWithSameNameByName(const string& input, string& output);
	bool addUpdateAuthor(const string& input, string& output);
	bool mergeAuthor(const string& input, string& output);
	bool fakeMergeAuthor(const string& input, string& output);
	bool searchAuthorRelation(const string& input, string& output);
	bool movePublication(const string& input, string& output);
	bool removeAuthors(const string& input, string& output);
	bool searchAuthorProfile(const string& input, string& output);
	bool authorSearchSuggest(const string& input, string& output);
	bool reloadAuthors(const string& input, string& output);
	bool recalcuFeatures(const string& input, string& output);
}