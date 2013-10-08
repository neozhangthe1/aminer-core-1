#include <boost/format.hpp>
#include "MergedAuthorDao.h"
#include "ConnectionPool.hpp"

string MergedAuthorDao::sql_fetch("select from_naid,to_naid from na_author_map");


void MergedAuthorDao::load_into(unordered_map<int,int>& id_map)
{
	Connection* conn = NULL;
	Statement* stat = NULL;
	ResultSet* rs = NULL;
	int from, to;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stat = conn->createStatement();
		rs = stat->executeQuery(sql_fetch);
		while(rs->next())
		{
			from = rs->getInt(1);
			to = rs->getInt(2);
			id_map[from] = to;
		}
		ConnectionPool::close(conn, stat, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

void MergedAuthorDao::add(vector<int>& froms, int to)
{
	Connection* conn = NULL;
	PreparedStatement* stat = NULL;

	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		conn->setAutoCommit(false);
		stat = conn->prepareStatement("insert into na_author_map(from_naid,to_naid) values(?,?)");
		stat->setInt(2, to);
		for (int from : froms)
		{
			stat->setInt(1, from);
			stat->executeUpdate();
		}
		conn->commit();
		conn->setAutoCommit(true);
		ConnectionPool::close(conn, stat, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}
