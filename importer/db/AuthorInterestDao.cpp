#include "AuthorInterestDao.h"
#include "util/Logger.h"

InterestItem::InterestItem(int _id, int _naid, const string& _interest, int _year)
	:id(_id),
	naid(_naid),
	year(_year),
	interest(_interest)
{
}

InterestItem::InterestItem()
{
}


AuthorInterestDao::AuthorInterestDao(void)
	:TABLE_NAME("person_interest"),
	//sql_fetchFrom("select id, aid, interest, year from person_interest where id>%1% and (year = -1 or year = 1) order by id limit %2%")
	sql_fetchFrom("select id, aid, interest, year from person_interest where id>%1% order by id limit %2%")		
{
	
}

AuthorInterestDao::~AuthorInterestDao(void)
{
}

vector<InterestItem*>* AuthorInterestDao::fetchFrom(int id, int limit)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;
	vector<InterestItem*>* models = NULL;
	try
	{
		string sql = str(boost::format(sql_fetchFrom) % id % limit);
		conn = ConnectionPool::getInstance()->getConnection();

		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);

		models = new vector<InterestItem*>();

		while(rs->next())
		{
			models->push_back(new InterestItem(rs->getInt(1),
				rs->getInt(2),//naid
				rs->getString(3),//interest
				rs->getInt(4)));//year
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return models;
}

vector<InterestItem> AuthorInterestDao::getAuthorInterest(int naid)
{
	vector<InterestItem>  items;
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;
	vector<InterestItem*>* models = NULL;
	try
	{
		string sql = str(boost::format("select interest, year from person_interest where aid=%1% order by year") % naid);
		conn = ConnectionPool::getInstance()->getConnection();

		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);

		InterestItem item;
		while(rs->next())
		{
			item.interest = rs->getString(1);
			item.year = rs->getInt(2);
			items.push_back(item);
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return std::move(items);
}

int AuthorInterestDao::count() throw(sql::SQLException)
{
	int value = 0;
	string sql = "select count(*) from " + TABLE_NAME;
	Connection* conn = ConnectionPool::getInstance()->getConnection();
	PreparedStatement* ps = conn->prepareStatement(sql);

	ResultSet* rs = ps->executeQuery();
	if(rs->next())
	{
		value = rs->getInt(1);
	}

	ConnectionPool::close(conn, ps, rs);
	return value;
}

void AuthorInterestDao::foreach(const std::function<void(InterestItem*)>& callback)
{
	int limit = 100000;

	int count = 0;
	int nStartID = -1;
	while(true)
	{
		vector<InterestItem*>* models = fetchFrom(nStartID, limit);

		if( NULL!=models && models->size()>0)
		{
			for(vector<InterestItem*>::iterator iter = models->begin(); iter!=models->end(); ++iter)
			{
				nStartID = (*iter)->id;
				callback(*iter);
			}
		}
		else
		{
			delete models;
			break;
		}
		++count;
		delete models;
	}
}

void AuthorInterestDao::updare_or_insert(int naid, const string& interest, int year)
{
	Connection* conn = NULL;
	PreparedStatement* stmt = NULL;
	ResultSet* rs = NULL;
	string sql_update = "update person_interest set interest=? where aid=? and year=?";
	string sql_insert = "insert into person_interest(aid, interest, year) values(?,?,?)";
	string sql_get_id = "select id from person_interest where aid=? and year=?";
	int id = -1;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		CHECK_PTR_LOG_FATAL(conn)
		{
			goto clean_up;
		}
		
		stmt = conn->prepareStatement(sql_get_id);
		stmt->setInt(1, naid);
		stmt->setInt(2, year);
		rs = stmt->executeQuery();
		if(rs->next())
			id = rs->getInt(1);
		if (stmt) {
			delete stmt;
			stmt = NULL;
		}

		if(id!=-1)
		{
			//update
			stmt = conn->prepareStatement(sql_update);
			CHECK_PTR_LOG_FATAL(stmt)
			{
				goto clean_up;
			}
			stmt->setString(1, interest);
			stmt->setInt(2, naid);
			stmt->setInt(3, year);

			stmt->executeUpdate();
		}
		else
		{//try insert
			stmt = conn->prepareStatement(sql_insert);
			CHECK_PTR_LOG_FATAL(stmt){goto clean_up;}
			stmt->setInt(1, naid);
			stmt->setString(2, interest);
			stmt->setInt(3, year);
			stmt->executeUpdate();
		}
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
clean_up:
	if(conn)
	{
		ConnectionPool::close(conn, stmt, rs);
	}
}
