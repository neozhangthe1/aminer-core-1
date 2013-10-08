#pragma once
#include"MassDataAdapter.h"
class ContactInfo
{
public:
	ContactInfo();
public:
	int contactId;
	//string name;
	string phone;
	string fax;
	string email;
	string homepage;
	string affiliation;
	string address;
	string position;
	string imgurl;
	string imgsrc;
	string imgname;
	string phduniv;
	string phdmajor;
	string phddate;
	string msuniv;
	string msmajor;
	string msdate;
	string bsuniv;
	string bsmajor;
	string bsdate;
	string bio;
};

class ContactInfoAdapter: public MassDataAdapter<ContactInfo>
{
public:
	static const string TABLE_NAME;
	static const string ID_FIELD;

 	static  ContactInfoAdapter& instance();
	virtual ~ContactInfoAdapter(void);
	DocumentCollection* getDocCollection();
public:
	int insert(ContactInfo& model);
	void update(ContactInfo& model);
protected:
	ContactInfoAdapter(void);
	virtual ContactInfo* _composeModel(ResultSet* rs);
	virtual long getID(ContactInfo* pub);
};