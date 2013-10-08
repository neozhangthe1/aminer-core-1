#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cppconn/statement.h>
#include <boost/format.hpp>
#include "ConnectionPool.hpp"
#include "Logger.h"

using std::vector;
using std::string;

class InterestItem
{
public:
	InterestItem(int _id, int _naid, const string& _interest, int _year);
	InterestItem();
public:
	int id;
	int naid;
	string interest;
	int year;
};

class AuthorInterestDao
{
public:
	string TABLE_NAME;

	AuthorInterestDao(void);

	~AuthorInterestDao(void);

	vector<InterestItem*>* fetchFrom(int id, int limit);
	vector<InterestItem> getAuthorInterest(int naid);
	int count() throw(sql::SQLException);

	void foreach(const std::function<void(InterestItem*)>& callback);

	void updare_or_insert(int naid, const string& interest, int year);
private:
	string sql_fetchFrom;

};
