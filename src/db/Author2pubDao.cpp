#include <string>
#include <vector>
#include <cppconn/prepared_statement.h>
#include <boost/format.hpp>
#include "Author2pubDao.h"
#include "ConnectionPool.hpp"
#include "util/Logger.h"

Author2pub::Author2pub(int _id, int _aid, int _pid, int _position)
		:id(_id),
		aid(_aid),
		pid(_pid),
		position(_position)
{
}

Author2pubDao::Author2pubDao()
	:TABLE_NAME("na_author2pub"),
	ABBR_NAME("na2p"),
	FIELD_LIST("na2p.id, na2p.aid, na2p.pid, na2p.position"),
	sql_fetchFrom("Select na2p.id, na2p.aid, na2p.pid, na2p.position from na_author2pub na2p where na2p.id > %1% order by na2p.id limit %2%"),
	sql_batchInsert("INSERT IGNORE INTO na_author2pub(aid, pid, position) values(?, ?, ?)")
{

}

Author2pubDao::~Author2pubDao()
{
}

vector<Author2pub*> Author2pubDao::fetchFrom(int id, int limit)
{
	Connection* conn = NULL;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;
	vector<Author2pub*> models;
	try
	{
		string sql = str(boost::format(sql_fetchFrom) % id % limit);
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();

		rs = stmt->executeQuery(sql);

		Author2pub* model = NULL;

		while(rs->next())
		{
			model = constructModel(rs);
			models.push_back(model);
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

vector<int> Author2pubDao::getAuthorPubs(int naid)
{
	Connection* conn = NULL;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;
	vector<int> pub_ids;
	try
	{
		string sql = str(boost::format("Select pid from na_author2pub where aid = %d") % naid);
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();
		rs = stmt->executeQuery(sql);
		
		pub_ids.reserve(rs->rowsCount());

		while(rs->next())
		{
			pub_ids.push_back(rs->getInt(1));
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return std::move(pub_ids);
}

vector<int> Author2pubDao::gerPubAuthors(int pub_id)
{
	Connection* conn = NULL;
	Statement *stmt = NULL;
	ResultSet* rs = NULL;
	vector<int> author_ids;
	try
	{
		string sql = str(boost::format("Select aid, position from na_author2pub where pid = %d order by position") % pub_id);
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();
		rs = stmt->executeQuery(sql);
		
		author_ids.reserve(rs->rowsCount());
		int position, aid;
		while(rs->next())
		{
			aid = rs->getInt(1);
			position = rs->getInt(2)-1;
			while(position>author_ids.size())
				author_ids.push_back(-1);
			author_ids.push_back(aid);
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return std::move(author_ids);
}

int Author2pubDao::count()
{
	int value = 0;
	string sql = "select count(*) from " + TABLE_NAME;
	Connection* conn = ConnectionPool::getInstance()->getConnection();
	Statement *stmt = NULL;

	stmt = conn->createStatement();
	ResultSet* rs = stmt->executeQuery(sql);
	if(rs->next())
	{
		value = rs->getInt(1);
	}

	ConnectionPool::close(conn, stmt, rs);

	return value;
}

int Author2pubDao::batchInsert(const vector<Author2pub>& a2ps)
{
	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	int sum = 0, temp;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		conn->setAutoCommit(false);

		ps = conn->prepareStatement(sql_batchInsert);
		for(auto iter = a2ps.begin(); iter!=a2ps.end(); ++iter)
		{
			ps->setInt(1, (*iter).aid);
			ps->setInt(2, (*iter).pid);
			ps->setInt(3, (*iter).position);
			temp = ps->executeUpdate();
			sum += temp;
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
	return sum;
}

void Author2pubDao::walk(const std::function<void(Author2pub*)>& callback)
{
    int limit = 100000;
	int count = 0;
	int nStartID = -1;
	while(true)
	{
		LOG(INFO) << boost::str(boost::format("fetching from %1%, limit %2%") % nStartID % limit);
		const vector<Author2pub*>& models = fetchFrom(nStartID, limit);
        if (models.size() == 0) break;

        for (auto a2p : models)
        {
            nStartID = a2p->id;
            callback(a2p);
        }
		++count;
	}
}

void Author2pubDao::move(int from, int to, int pub_id)
{
	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	int sum = 0, temp;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		ps = conn->prepareStatement("update na_author2pub set aid = ? where pid = ? and aid = ?");
		ps->setInt(1, to);
		ps->setInt(2, pub_id);
		ps->setInt(3, from);
		ps->execute();
		ConnectionPool::close(conn, ps, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

void Author2pubDao::move(int to, vector<Author2pub>& pubs)
{
	Connection* conn = NULL;
	PreparedStatement* ps = NULL;
	int sum = 0, temp;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		conn->setAutoCommit(false);

		ps = conn->prepareStatement("update na_author2pub set aid = ? where pid = ? and aid = ?");
		for (Author2pub& item : pubs)
		{
			ps->setInt(1, to);
			ps->setInt(2, item.pid);
			ps->setInt(3, item.aid);
			temp = ps->executeUpdate();
			sum += temp;
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
	if(sum!=pubs.size())
	{
		LOG(ERROR) << boost::str(boost::format("Error happened at %1%, %2%") % __FILE__ % __LINE__);
	}
}

void Author2pubDao::remove_author(int naid)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	int sum = 0, temp;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();
		const string& query = boost::str(boost::format("delete from na_author2pub where aid=%d") % naid);

		int count = stmt->executeUpdate(query);
		ConnectionPool::close(conn, stmt, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

void Author2pubDao::remove_pub(int pub_id)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	int sum = 0, temp;
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->createStatement();
		const string& query = boost::str(boost::format("delete from na_author2pub where pid=%d") % pub_id);

		int count = stmt->executeUpdate(query);
		ConnectionPool::close(conn, stmt, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

Author2pub* Author2pubDao::constructModel(ResultSet* rs)
{
	Author2pub* model = NULL;
	try
	{
		model = new Author2pub(
			rs->getInt(1),
			rs->getInt(2),
			rs->getInt(3),
			rs->getInt(4)
			);

	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return model;
}

GetPub2AuthorMapFunc::GetPub2AuthorMapFunc()
{
	p2a = new P2AMap();
	// TODO: get a solid number, and read it from config file
	p2a->rehash(4000000);

}

GetPub2AuthorMapFunc::~GetPub2AuthorMapFunc()
{
	for(auto iter : *p2a)
	{
        for (auto viter : iter.second)
        {
			delete viter;
		}
	}
	delete p2a;
	LOG(INFO) << "~GetPub2AuthorMapFunc\n";
}

void GetPub2AuthorMapFunc::operator()(Author2pub* model)
{
	P2AMap::iterator iter = p2a->find(model->pid);
	if(iter == p2a->end())
	{
		iter = p2a->insert(make_pair(model->pid, vector<Author2pub*>())).first;
		iter->second.resize(10, NULL);
		this->max_position[model->pid]=0;
	}
	if(model->position>=iter->second.size())
	{
		iter->second.resize(model->position*2,NULL);
	}
	if(model->position>this->max_position[model->pid])
		this->max_position[model->pid] = model->position;

	iter->second[model->position] = model;
	if(model->id%100000==0)
		LOG(INFO) << boost::str(boost::format("%1% -- %2% totalsize:%3%") % model->aid % model->pid % p2a->size());
}

vector<Author2pub*> GetPub2AuthorMapFunc::getAuthorPubByPID(int pID)
{
	P2AMap::iterator iter = p2a->find(pID);
	if(iter == p2a->end())
		return vector<Author2pub*>();
	return iter->second;
}

vector<int> GetPub2AuthorMapFunc::getAuthorIDsByPID(int pID)
{
	P2AMap::iterator a2piter = p2a->find(pID);
	vector<int> rs;
	if(a2piter != p2a->end())
    {
        rs.reserve(a2piter->second.size());
        for(int i=1;i<=this->max_position[pID];++i)
        {
            Author2pub* ptr = a2piter->second[i];
            if(ptr)
            {
                rs.push_back(ptr->aid);
            }
            else
            {
                rs.push_back(-1);
            }
        }
    }
	return rs;
}
