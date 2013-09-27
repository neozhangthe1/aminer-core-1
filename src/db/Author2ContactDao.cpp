#include <mutex>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "Author2ContactDao.h"
#include "Logger.h"

Author2ContactDao::Author2ContactDao(void)
	:TABLE_NAME("na_person"),
	sql_fetchFrom("select id, names, contact_id, type from na_person where id>%1% order by id limit %2%"),
	sql_get_item("select id, names, contact_id, type from na_person where id=%1%")
{	

}

Author2ContactDao::~Author2ContactDao(void)
{
}

vector<NapersonItem*>* Author2ContactDao::fetchFrom(int id, int limit)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;
	vector<NapersonItem*>* models = NULL;
	NapersonItem* model = NULL;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		string sql = str(boost::format(sql_fetchFrom) % id % limit);

		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);

		models = new vector<NapersonItem*>();
		models->reserve(limit);

		int contactid = -1;
		while(rs->next())
		{
			contactid = rs->getInt(3);
			/*if(contactid!=-1)
			{*/
			model = new NapersonItem();
			model->naid = rs->getInt(1);
			model->contactid = contactid;
			model->type = rs->getInt(4);
            string temp = rs->getString(2);
			boost::split(model->names, temp, boost::is_any_of(","), boost::token_compress_on);
			//this->splitNameAlias(rs->getString(2), model->name, model->alias);
			models->push_back(model);
			//}
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

int Author2ContactDao::getMaxId() throw(sql::SQLException)
{
	int value = 0;
	string sql = "select max(id) from " + TABLE_NAME;
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

int Author2ContactDao::count() throw(sql::SQLException)
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

void Author2ContactDao::foreach(const std::function<void(NapersonItem*)>& callback)
{
	int limit = 100000;

	int count = 0;
	int nStartID = -1;
	while(true)
	{
		vector<NapersonItem*>* models = fetchFrom(nStartID, limit);

		if( NULL!=models && models->size()>0)
		{
			for(vector<NapersonItem*>::iterator iter = models->begin(); iter!=models->end(); ++iter)
			{
				nStartID = (*iter)->naid;
				callback(*iter);
			}
			LOG(INFO) << boost::str(boost::format("loaded %1% and the nStartID is %2%") % models->size() % nStartID);
			delete models;
		}
		else
		{
			delete models;
			break;
		}
		++count;
	}
}

NapersonItem Author2ContactDao::getItem(int id)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;
	NapersonItem model;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		string sql = str(boost::format(sql_get_item) % id);

		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);

		int contactid = -1;
		while(rs->next())
		{
			contactid = rs->getInt(3);
			model.naid = rs->getInt(1);
			model.contactid = contactid;
			model.type = rs->getInt(4);
            string temp = rs->getString(2);
			boost::split(model.names, temp, boost::is_any_of(","), boost::token_compress_on);
			//this->splitNameAlias(rs->getString(2), model.name, model.alias);
			
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return model;
}

int Author2ContactDao::addItem(const string& names, int contactId, int type)
{
	static std::mutex _mutex;
	//make sure only one thread could add author
	std::lock_guard<std::mutex> lock(_mutex);

	int naid = this->getMaxId()+1;
	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();

		ps = conn->prepareStatement("insert into na_person(id, names, contact_id, type) values (?,?,?,?)");
		ps->setInt(1, naid);
		ps->setString(2, names);
		ps->setInt(3, contactId);
		ps->setInt(4, type);
		ps->execute();
		ConnectionPool::close(conn, ps, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return naid;
}

void Author2ContactDao::updateItem(int naid, string& names, int contactId, int type)
{
	if(naid<0)
		return;

	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();

		string sql;
		sql.reserve(100);
		sql = "update na_person set ";
		if(!names.empty())
			sql+="names=?,";
		if(contactId!=-1)
			sql+="contact_id=?,";
		if(type!=-1)
			sql+="type=?";
		else
			sql.erase(sql.end()-1);
		sql+=" where id=?";

		ps = conn->prepareStatement(sql);
		int i=0;
		if(!names.empty())
			ps->setString(++i, names);
		if(contactId!=-1)
			ps->setInt(++i, contactId);
		if(type!=-1)
			ps->setInt(++i, type);
		ps->setInt(++i, naid);
		ps->execute();
		ConnectionPool::close(conn, ps, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

void Author2ContactDao::removeItem(vector<int>& ids)
{
	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		conn->setAutoCommit(false);
		ps = conn->prepareStatement("delete from na_person where id=?");
		for (int id : ids)
		{
			ps->setInt(1,id);
			ps->executeUpdate();
		}
		conn->commit();
		conn->setAutoCommit(true);
		ConnectionPool::close(conn, ps, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

void Author2ContactDao::splitNameAlias(string& names,string& name, string& alias)
{
	size_t start =  names.find(',');
	if(start!=string::npos)
	{
		name = names.substr(0, start);
		alias = names.substr(start+1, names.size()-start-1);
	}
	else
	{
		name = names;
	}
}

