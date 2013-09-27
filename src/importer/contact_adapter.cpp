#include"ContactInfoAdapter.h"
#include "DbUtil.h"

ContactInfo::ContactInfo()
{//do nothing
}

//Initialize static memebers of ContactInfoAdapter
const string ContactInfoAdapter::TABLE_NAME = "contact_info";
const string ContactInfoAdapter::ID_FIELD = "id";

ContactInfoAdapter&  ContactInfoAdapter::instance()
{
	static  ContactInfoAdapter _instance;
	return _instance;
}

ContactInfoAdapter::ContactInfoAdapter()
{
	MassDataAdapter<ContactInfo>::tableName = TABLE_NAME;
	MassDataAdapter<ContactInfo>::idFieldName = ID_FIELD;

	MassDataAdapter<ContactInfo>::queryFields[FETCHTYPE_FULL] = "id,phone, fax, email,homepage,affiliation,address,position, phduniv, phdmajor, phddate, msuniv, msmajor, msdate, bsuniv, bsmajor, bsdate, imgsrc, imgurl, imgname, bio";
	
	MassDataAdapter<ContactInfo>::orderBySqls.assign(NUM_TYPES,"id");

	MassDataAdapter<ContactInfo>::setFetchType(FETCHTYPE_FULL);
}

ContactInfoAdapter::~ContactInfoAdapter()
{
#ifdef SERVERDB
	printf("ContactInfoAdapter::~ContactInfoAdapter\n");
#endif
}

ContactInfo* ContactInfoAdapter::_composeModel(ResultSet* rs)
{
	ContactInfo* info = NULL;
	try
	{
		info = new ContactInfo();
		int i=0;
		info->contactId = rs->getInt(++i);
		info->phone = rs->getString(++i);
		info->fax = rs->getString(++i);
		info->email = rs->getString(++i);
		info->homepage = rs->getString(++i);
		info->affiliation = rs->getString(++i);
		info->address = rs->getString(++i);
		info->position = rs->getString(++i);
		info->phduniv = rs->getString(++i);
		info->phdmajor = rs->getString(++i);
		info->phddate = rs->getString(++i);
		info->msuniv = rs->getString(++i);
		info->msmajor = rs->getString(++i);;
		info->msdate = rs->getString(++i);
		info->bsuniv = rs->getString(++i);
		info->bsmajor = rs->getString(++i);;
		info->bsdate = rs->getString(++i);
		info->imgsrc = rs->getString(++i);
		info->imgurl = rs->getString(++i);
		info->imgname = rs->getString(++i);
		info->bio = rs->getString(++i);
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
	}
	return info;
}

long ContactInfoAdapter::getID(ContactInfo* pub)
{
	return pub->contactId;
}

int ContactInfoAdapter::insert(ContactInfo& model)
{
	int inserted_id = -1;
	Connection* conn = NULL;
	PreparedStatement* stmt = NULL;
	string sql = "insert into contact_info(phone, fax, email,homepage,affiliation, \
		address,position, phduniv, phdmajor, phddate, \
		msuniv, msmajor, msdate, bsuniv, bsmajor, \
		bsdate, imgsrc, imgurl, imgname, bio) \
		values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->prepareStatement(sql);
		
		int i=0;
		stmt->setString(++i, model.phone);
		stmt->setString(++i, model.fax);
		stmt->setString(++i, model.email);
		stmt->setString(++i, model.homepage);
		stmt->setString(++i, model.affiliation);
		stmt->setString(++i, model.address);
		stmt->setString(++i, model.position);
		stmt->setString(++i, model.phduniv);
		stmt->setString(++i, model.phdmajor);
		stmt->setString(++i, model.phddate);
		stmt->setString(++i, model.msuniv);
		stmt->setString(++i, model.msmajor);
		stmt->setString(++i, model.msdate);
		stmt->setString(++i, model.bsuniv);
		stmt->setString(++i, model.bsmajor);
		stmt->setString(++i, model.bsdate);
		stmt->setString(++i, model.imgsrc);
		stmt->setString(++i, model.imgurl);
		stmt->setString(++i, model.imgname);
		stmt->setString(++i, model.bio);
		
		stmt->executeUpdate();

		inserted_id = DbUtil::last_insert_id(conn);
		ConnectionPool::close(conn, stmt, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
	return inserted_id;
}

void ContactInfoAdapter::update(ContactInfo& model)
{
	Connection* conn = NULL;
	PreparedStatement* stmt;
	string sql = "update contact_info set phone=?, fax=?, email=?, homepage=?, affiliation=?, \
		address=?,position=?, phduniv=?, phdmajor=?, phddate=?, \
		msuniv=?, msmajor=?, msdate=?, bsuniv=?, bsmajor=?, \
		bsdate=?, imgsrc=?, imgurl=?, imgname=?, bio=? \
		where id=?";
	try
	{
		conn = ConnectionPool::getInstance()->getConnection();
		stmt = conn->prepareStatement(sql);
		
		int i=0;
		stmt->setString(++i, model.phone);
		stmt->setString(++i, model.fax);
		stmt->setString(++i, model.email);
		stmt->setString(++i, model.homepage);
		stmt->setString(++i, model.affiliation);
		stmt->setString(++i, model.address);
		stmt->setString(++i, model.position);
		stmt->setString(++i, model.phduniv);
		stmt->setString(++i, model.phdmajor);
		stmt->setString(++i, model.phddate);
		stmt->setString(++i, model.msuniv);
		stmt->setString(++i, model.msmajor);
		stmt->setString(++i, model.msdate);
		stmt->setString(++i, model.bsuniv);
		stmt->setString(++i, model.bsmajor);
		stmt->setString(++i, model.bsdate);
		stmt->setString(++i, model.imgsrc);
		stmt->setString(++i, model.imgurl);
		stmt->setString(++i, model.imgname);
		stmt->setString(++i, model.bio);

		stmt->setInt(++i, model.contactId);

		int rs = stmt->executeUpdate();
		if(rs != 1)
		{
			LOG(INFO) << boost::str(boost::format("No record was changed while updating contact_info where id = %d") % model.contactId);
		}
		ConnectionPool::close(conn, stmt, NULL);
	}
	catch (sql::SQLException &e) 
	{
		LOG(ERROR) << boost::str(boost::format("# ERR: SQLException in  %1% %2%") % __FILE__ %__FUNCTION__);
		LOG(ERROR) << boost::str(boost::format("%1% error code %2%") %e.what() % e.getErrorCode());
	}
}