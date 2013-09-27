#pragma once
#include <string>
#include <vector>
#include <functional>
#include "ConnectionPool.hpp"
#include "Logger.h"

class NapersonItem
{
public:
	NapersonItem()
		:naid(-1)
	{
	}
	int naid;
	vector<string> names;
	//string alias;
	int contactid;
	int type;
};

class Author2ContactDao
{
public:
	string TABLE_NAME;

	Author2ContactDao(void);
	Author2ContactDao(const Author2ContactDao&) = delete;

	~Author2ContactDao(void);

	vector<NapersonItem*>* fetchFrom(int id, int limit);

	int count() throw(sql::SQLException);

	void foreach(const std::function<void(NapersonItem*)>& callback);

	NapersonItem getItem(int id);

	int getMaxId() throw(sql::SQLException);

	//The naid of the added author is returned
	int addItem(const string& names, int contactId = -1, int type = -1);

	void updateItem(int naid, string& names, int contactId = -1, int type = -1);

	void removeItem(vector<int>& ids);
private:
	void splitNameAlias(string& names,string& name, string& alias);
	string sql_fetchFrom;
	string sql_get_item;

};

