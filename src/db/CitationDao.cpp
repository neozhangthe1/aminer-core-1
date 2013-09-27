#include <boost/format.hpp>
#include "CitationDao.h"
#include "DbUtil.h"

CitationDao:: CitationDao()
	:sql_fetch("select citepubkey, citedpubkey, id from citation where id> %1% order by id limit %2%")
{

}

void  CitationDao::foreach(const std::function<void(ResultSet*)>& callback)
{
	Connection* conn = NULL;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;
	int start_id = -1, limit = 100000;
	bool flag = true;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		CHECK_PTR_LOG_FATAL(conn){goto stop;}
		stmt = conn->createStatement();
		CHECK_PTR_LOG_FATAL(stmt){goto stop;}
		while(flag)
		{
			flag = false;
			rs = stmt->executeQuery(boost::str(boost::format(sql_fetch) % start_id % limit));
			CHECK_PTR_LOG_FATAL(rs){goto stop;}
			while(rs->next())
			{
				start_id = DbUtil::atoi(rs->getString(3).c_str());
				callback(rs);
				flag = true;
				if(start_id % 100000 == 0)
					LOG(INFO) << boost::str(boost::format("citation #%d") % start_id);
			}
			if (rs) {
				delete rs;
				rs = NULL;
			}
		}
	}
	catch (sql::SQLException &e)
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  \n%1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
stop:
    ConnectionPool::close(conn, stmt, rs);
}

void CitationDao::remove_pub(const string& pub_key)
{
	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		CHECK_PTR_LOG_FATAL(conn){goto stop;}
		ps = conn->prepareStatement("delete from citation where citepubkey=? or citedpubkey=?");
		CHECK_PTR_LOG_FATAL(ps){goto stop;}
		ps->setString(1, pub_key);
		ps->setString(2, pub_key);
		int count = ps->executeUpdate();
		ConnectionPool::close(conn, ps, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  \n%1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
stop:
    ConnectionPool::close(conn, ps, NULL);
}
