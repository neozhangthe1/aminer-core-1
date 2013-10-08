#pragma once
#include <functional>
#include "ConnectionPool.hpp"
#include "Logger.h"

class CitationDao
{
public:
	CitationDao();
	void foreach(const std::function<void(ResultSet*)>& callback);
	void remove_pub(const string& pub_key);
private:
	string sql_fetch;

};
