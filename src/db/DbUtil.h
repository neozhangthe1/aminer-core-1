#pragma once
#include <mutex>
#include <string>
#include <cppconn/resultset.h>
#include <cppconn/connection.h>
#include "Logger.h"
using std::string;

class DbUtil
{
public:
	// 此函数用于提高string到数值的转换效率。假设数据均是符合要求的。
	static int atoi(const char* input);
	static double atof(const char* input);
	static int last_insert_id(sql::Connection* conn);
};


class MultithreadDBFetcher
{
public:
	MultithreadDBFetcher(int thread_count, string& sql, const std::function<void(sql::ResultSet*)>& callback, int limit = 10000);
	void operator()();
	void start();
private:
	MultithreadDBFetcher(const MultithreadDBFetcher&);
	string next_sql();
private:
	int thread_count;
	string sql;
    std::function<void(sql::ResultSet*)> callback;
	int limit;
	int offset;
	std::mutex offset_mutex;
};
