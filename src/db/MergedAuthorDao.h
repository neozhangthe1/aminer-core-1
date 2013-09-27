#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include "Logger.h"
using std::vector;
using std::string;
using std::unordered_map;

class MergedAuthorDao
{
public:
	void load_into(unordered_map<int,int>& id_map);
	void add(vector<int>& from, int to);
private:
	static string sql_fetch;

};
