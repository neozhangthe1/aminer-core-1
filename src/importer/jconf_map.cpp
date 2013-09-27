#include <boost/format.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include "JConfMap.h"
#include "ConnectionPool.hpp"
#include "stringutils.h"
#include "util/Logger.h"

using namespace sql;
JournalConferenceInfo::JournalConferenceInfo()
{
	//do nothing
}


JournalConferenceInfo::JournalConferenceInfo(int _ID, string& _name, double _score, int _index, double _percent, string& _alias)
	:ID(_ID),
	name(_name),
	score(_score),
	index(_index),
	percent(_percent),
	alias(_alias)
{
}

JConfMap::JConfMap()
{

	load();
	this->loadImpact();
}
JConfMap::~JConfMap()
{

}
JConfMap* JConfMap::getInstance()
{
	static JConfMap instance;

	return &instance;
}

string JConfMap::getName(int id)
{
	if(id != -1)
	{
		auto iter = jconfhash.find(id);

		if(iter == jconfhash.end())
		{
			return "";
		}
		return iter->second->name;
	}
	return "";
}

string JConfMap::getAlias(int id)
{
	if(id != -1)
	{
		auto iter = jconfhash.find(id);

		if(iter == jconfhash.end())
		{
			return "";
		}
		return iter->second->alias;
	}
	else
		return "";
}

double JConfMap::getScore(int id)
{
	if(id != -1)
	{
		auto iter = jconfhash.find(id);

		if(iter == jconfhash.end())
		{
			return 0.0;
		}
		return iter->second->score;
	}
	else
		return 0.0;
}

double JConfMap::getScoreByName(const string& name)
{
	int id = getIdByName(name);
	return getScore(id);
}

int JConfMap::getIdByName(string name)
{
	strutils::trim(name);
	if(name.length()>0)
	{
		strutils::to_lower(name);
		auto iter = jconfnamehash.find(name);

		if(iter!= jconfnamehash.end())
		{
			return iter->second->ID;
		}
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

bool JConfMap::containsConf(string name)
{
	strutils::trim(name);
	if(name.length()==0)
		return false;
	auto iter = jconfnamehash.find(name);

	if(iter!= jconfnamehash.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

JournalConferenceInfo* JConfMap::getJConf(int id)
{
	if(id != -1)
	{
		auto iter = jconfhash.find(id);

		if(iter == jconfhash.end())
		{
			return NULL;
		}
		return iter->second;
	}
	else
		return NULL;
}

int JConfMap::getSize()
{
	return jconfhash.size();
}

void JConfMap::destroy()
{
	for (auto iter = jconfhash.begin(); iter != jconfhash.end(); ++iter)
	{
		delete iter->second;
	}
}

double JConfMap::getImpact(int id)
{
	auto iter = this->impact_factor.find(id);
	if(iter != this->impact_factor.end())
		return iter->second;
	return 0.0;
}

void JConfMap::load()
{
	Connection* conn = NULL;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;

	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();

		rs = stmt->executeQuery(this->sql);
		int id = -1;
		size_t count;
		string name;
		string alias;
		double score = 0.0;

		typedef pair<int, JournalConferenceInfo*> Int_JConf;
		typedef pair<string, JournalConferenceInfo*> Str_JConf;

		// rehash to prevent memory copy
		count = rs->rowsCount();
		this->jconfhash.rehash(count);
		this->jconfnamehash.rehash(count);

		while(rs->next())
		{
			id = rs->getInt(1);
			name = rs->getString(2);
			alias = rs->getString(3);
			score = rs->getDouble(4);
			if(score<0.05)
				score = 0.05;
			JournalConferenceInfo* jconf = new JournalConferenceInfo(id, name, score, -1, 0.0, alias);
			jconfhash.insert(Int_JConf(id, jconf));

			strutils::trim(name);
			strutils::to_lower(name);
			jconfnamehash.insert(Str_JConf(name, jconf));

			strutils::trim(alias);
			if(alias.length()>0)
			{
				strutils::to_lower(alias);
				jconfnamehash.insert(Str_JConf(alias, jconf));
			}
		}
		ConnectionPool::close(conn,stmt,rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	LOG(INFO) << boost::str(boost::format("JConfMap init finish Size: %1%") % jconfhash.size());
}

void JConfMap::loadImpact()
{
	Connection* conn = NULL;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;
	string sql_str("select conf_id, score from conference_rank where year=0 and (type=5 or type=6)");
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();

		rs = stmt->executeQuery(sql_str);

		// rehash to prevent memory copy
		size_t count = rs->rowsCount();
		this->impact_factor.rehash(count);
		
		while(rs->next())
		{
			this->impact_factor[rs->getInt(1)]=rs->getDouble(2);
		}
		ConnectionPool::close(conn,stmt,rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	LOG(INFO) << boost::str(boost::format("JConfMap Impact factor init finish Size: %1%") % impact_factor.size());
}

bool JConfMap::getConfWithPrefix(vector<JournalConferenceInfo*>& results, string prefix)
{
	if(prefix.empty() || prefix[0]=='#')
	{
		for (auto pair : jconfhash)
		{
			results.push_back(pair.second);
		}
		goto end;
	}

	//deal with
	if(prefix[0]=='!')
	{
		for (auto pair : jconfnamehash)
		{
			if(!((pair.first[0]>='0'&&pair.first[0]<='9')||(pair.first[0]>='a'&&pair.first[0]<='z')||(pair.first[0]>='A'&&pair.first[0]<='Z')))
			{
				results.push_back(pair.second);
			}
		}
		goto end;
	}
	
	boost::to_lower(prefix);
	for (auto pair : jconfnamehash)
	{
		if(boost::starts_with(boost::to_lower_copy(pair.first), prefix))
		{
			results.push_back(pair.second);
		}
	}
end:
	return true;
}

const string JConfMap::tableName("jconf");
const string JConfMap::sql("select id, name, alias, score from jconf");
