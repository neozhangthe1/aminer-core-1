#include <string>
#include <vector>
#include <boost/format.hpp>
#include <cppconn/statement.h>
#include "AuthorFeatureDao.h"
#include "ConnectionPool.hpp"
#include "DbUtil.h"
#include "AuthorAdapter.h"

using std::vector;
using std::string;



AuthorFeatureDao::AuthorFeatureDao(void)
	:TABLE_NAME("person_ext"),
	//sql_fetchFrom("select id, person_id, type, score, rank from person_ext where topic=-1 and id>%1% order by id limit %2%")
	sql_fetchFrom("select person_id, topic,\
				  sum(case when `type`=0 then score else 0 end ) as PUBCOUN, sum(case when `type`=0 then rank else 0 end) as PUBCOUN_RANK, \
				  sum(case when `type`=1 then score else 0 end ) as CITATIONNO, sum(case when `type`=1 then rank else 0 end) as CITATIONNO_RANK, \
				  sum(case when `type`=2 then score else 0 end ) as HINDEX, sum(case when `type`=2 then rank else 0 end) as HINDEX_RANK, \
				  sum(case when `type`=3 then score else 0 end ) as ACTIVITY, sum(case when `type`=3 then rank else 0 end) as ACTIVITY_RANK, \
				  sum(case when `type`=4 then score else 0 end ) as UPTREND, sum(case when `type`=4 then rank else 0 end) as UPTREND_RANK, \
				  sum(case when `type`=5 then score else 0 end ) as NEWSTARSCORE, sum(case when `type`=5 then rank else 0 end) as NEWSTARSCORE_RANK, \
				  sum(case when `type`=6 then score else 0 end ) as LONGEVITY, sum(case when `type`=6 then rank else 0 end) as LONGEVITY_RANK, \
				  sum(case when `type`=7 then score else 0 end ) as DIVERSITY, sum(case when `type`=7 then rank else 0 end) as DIVERSITY_RANK, \
				  sum(case when `type`=8 then score else 0 end ) as SOCIABILITY, sum(case when `type`=8 then rank else 0 end) as SOCIABILITY_RANK, \
				  sum(case when `type`=9 then score else 0 end ) as GINDEX, sum(case when `type`=9 then rank else 0 end) as GINDEX_RANK, \
				  sum(case when `type`=103 then rank else 0 end) as MOSTCITEDPAPER \
				  from person_ext \
				  where person_id>%1% and person_id<=%2% \
				  group by person_id,topic"
				  ),
				  sql_update("replace into person_ext (person_id, type, topic, score, rank) \
							 values(?, ?, ?, ?, ?)")
{	
}

AuthorFeatureDao::~AuthorFeatureDao()
{
}

int AuthorFeatureDao::count() throw(sql::SQLException)
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

void AuthorFeatureDao::foreach(const function<void(ResultSet*)>& callback)
{
	MultithreadDBFetcher fetcher(8, this->sql_fetchFrom, callback, 1000);
	fetcher.start();
}

void AuthorFeatureDao::getAuthorFeature(Author& author)
{
	Connection* conn = NULL;
	Statement* stmt = NULL;
	ResultSet* rs = NULL;
	try
	{
		string sql = boost::str(boost::format(sql_fetchFrom) % (author.naid-1) % author.naid);
			
		conn = ConnectionPool::getInstance()->getConnection();
			
		stmt = conn->createStatement();
		stmt->setResultSetType(ResultSet::TYPE_FORWARD_ONLY);
		rs = stmt->executeQuery(sql);

		while(rs->next())
		{
			author.feature.add(rs);
		}
		ConnectionPool::close(conn, stmt, rs);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}

void AuthorFeatureDao::updateAuthorFeatureInDB(Author& author)
{
	AuthorFeatures& feature = author.feature;
	int naid = author.naid;
	for(int i=0;i<feature.expertiseTopic.size(); ++i)
	{
		int topic = feature.expertiseTopic[i];
		Connection* conn = NULL;
		PreparedStatement* stmt = NULL;
		ResultSet* rs = NULL;
		try
		{
			conn = ConnectionPool::getInstance()->getConnection();
			conn->setAutoCommit(false);
			stmt = conn->prepareStatement(this->sql_update);
			stmt->setInt(1, naid);
			stmt->setInt(3, topic);
			for(int type = 0;type<AuthorFeatures::TOTAL_FEATURES_WITH_TOPIC;++type)
			{
				stmt->setInt(2, type);
				stmt->setDouble(4, feature.scores[i][type]);
				stmt->setInt(5, feature.ranks[i][type]);
				stmt->executeUpdate();
			}
			conn->commit();
			conn->setAutoCommit(true);
			ConnectionPool::close(conn, stmt, NULL);
		}
		catch (sql::SQLException& e)
		{
			LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
			LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
		}
	}
}
