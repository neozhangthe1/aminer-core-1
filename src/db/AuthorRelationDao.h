#pragma once
#include <string>
#include <vector>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include "AuthorRelation.h"
#include "Logger.h"
using std::vector;
using std::string;

class AuthorRelationDao
{
public:
	string TABLE_NAME;
		
	AuthorRelationDao(void);
	~AuthorRelationDao(void);
	int foreach(int person_id, int limit, const std::function<void(sql::ResultSet*)>& callback);
	void foreach(const std::function<void(sql::ResultSet*)>& callback);
	vector<AuthorRelation> getRelations(int naid);
private:
	string sql_fetchFrom;

};
