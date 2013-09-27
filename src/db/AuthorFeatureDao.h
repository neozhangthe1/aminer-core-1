#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include "Logger.h"
using std::vector;
using std::string;

class Author;

class AuthorFeatureDao
{
public:
	string TABLE_NAME;

	AuthorFeatureDao(void);
	~AuthorFeatureDao(void);
	int count() throw(sql::SQLException);
	void foreach(const std::function<void(sql::ResultSet*)>& callback);
	void getAuthorFeature(Author& author);
	void updateAuthorFeatureInDB(Author& author);
private:
	string sql_fetchFrom;
	string sql_update;

};

