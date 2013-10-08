#include <string>
#include <vector>
#include <cppconn/statement.h>
#include <boost/format.hpp>
#include "AuthorRelationDao.h"
#include "ConnectionPool.hpp"
#include "DbUtil.h"

using std::vector;
using std::string;


AuthorRelationDao::AuthorRelationDao(void)
	:TABLE_NAME("na_person_relation"),
	//sql_fetchFrom("select id, person_id, type, score, rank from person_ext where topic=-1 and id>%1% order by id limit %2%")
	sql_fetchFrom("select id, pid1, pid2, similarity, rel_type from na_person_relation where id > %1% order by id limit %2%"
				  )
{	

}

AuthorRelationDao::~AuthorRelationDao()
{
}

int AuthorRelationDao::foreach(int person_id, int limit, const std::function<void(sql::ResultSet*)>& callback)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;

	int maxid = -1, temp;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		string sql = str(boost::format(sql_fetchFrom) % person_id % (person_id+limit));
		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);
		while(rs->next())
		{
			temp = rs->getInt(1);
			maxid = temp>maxid?temp:maxid;
			callback(rs);
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return maxid;
}

void AuthorRelationDao::foreach(const std::function<void(ResultSet*)>& callback)
{
	int limit = 10000;
	int count = 0;
	int nStartID = -1;
	do
	{
		nStartID = foreach(nStartID, limit, callback);
			
	}while(nStartID!=-1);
}


vector<AuthorRelation> AuthorRelationDao::getRelations(int naid)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;

	vector<AuthorRelation> relations;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		string sql = str(boost::format("select pid1, pid2, similarity, rel_type from na_person_relation where pid1 = %1% or pid2 = %1% order by similarity desc") % naid);
		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);
		while(rs->next())
		{
			AuthorRelation relation(rs->getInt(1), rs->getInt(2), rs->getDouble(3), rs->getString(4));
			if(relation.pid1!=naid)
			{
				std::swap(relation.pid1, relation.pid2);
			}
			relations.push_back(relation);
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return std::move(relations);
}
