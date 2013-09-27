#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <mysql/mysql.h>
#include "DbUtil.h"
#include "ConnectionPool.hpp"
#include "util/threads.h"



int DbUtil::atoi(const char* input)
{
	int result = 0;
	char c;

	while(c = *input)
	{
		result = result * 10 + (c - '0');
		input++;
	}

	return result;
}

double DbUtil::atof(const char* input)
{
	double result = 0;
	int signal = 1;
	bool isFraction = 0;
	int e = 0;
	char c;

	c = *input;
	if(c == '-' || c == '+')
	{
		if(c == '-')
		{
			signal = -1;
		}
		input++;
		c = *input;
	}

	while(c)
	{
		if(c=='.')
		{
			isFraction = true;
		}
		else
		{
			if(isFraction)
			{
				--e;
			}
			result = result * 10 + (c - '0');
		}
		input++;
		c = *input;
	}

	while(e)
	{
		result *= 0.1;
		++e;
	}
	result *= signal;

	return result;
}

int DbUtil::last_insert_id(Connection* conn)
{
	int result = -1;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;
	if(!conn)
		goto clean_up;	
	try
	{
		stmt = conn->createStatement();
		rs = stmt->executeQuery("SELECT LAST_INSERT_ID()");

		if(rs->next())
		{
			result = rs->getInt(1);
		}
        rs->close();
		delete rs;
        stmt->close();
		delete stmt;
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
clean_up:
	return result;
}

MultithreadDBFetcher::MultithreadDBFetcher(int _thread_count, string& _sql, const function<void(sql::ResultSet*)>& _callback, int _limit)
	:thread_count(_thread_count),
	sql(_sql),
	callback(_callback),
	limit(_limit)
{
	offset = -1;
}

void MultithreadDBFetcher::start()
{
    dispatch_thread_group(std::bind(&MultithreadDBFetcher::operator(), this), this->thread_count);
}

void MultithreadDBFetcher::operator()()
{
	int count;
	Connection* conn = ConnectionPool::getInstance()->getConnection();
	Statement* stmt = NULL;
	ResultSet* rs = NULL;

	do{
		count=0;
		string sql = this->next_sql();
		try
		{
            stmt = conn->createStatement();
            stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
            rs = stmt->executeQuery(sql);
            while(rs->next())
            {
                ++count;
                callback(rs);
            }
            rs->close();
            delete rs;
            rs = NULL;
            stmt->close();
            delete stmt;
            stmt = NULL;
		}
		catch (sql::SQLException &e)
		{
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
		}
	} while(count);
    ConnectionPool::close(conn, stmt, rs);
    // XXX solve this with a real connection pool.
    mysql_thread_end();
}

string MultithreadDBFetcher::next_sql()
{
	int num1,num2;
	{
		std::lock_guard<std::mutex> lock(offset_mutex);
		num1 = this->offset;
		num2 = num1+limit;
		this->offset = num2;
	}
	return boost::str(boost::format(this->sql) % num1 % num2);
}
