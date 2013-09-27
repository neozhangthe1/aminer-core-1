#pragma once
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include "document.h"
#include "AuthorFeatureDao.h"
#include "AuthorFeatures.h"
#include "AuthorRelation.h"
#include "ContactInfoAdapter.h"
#include "OrganizationAdapter.h"
#include "Logger.h"
using std::string;
using std::vector;

class InterestItem;
class Publication;
class NapersonItem;
class Author2pub;

class Author: public Document
{
public:

	enum Type
	{
		TYPE_REALPERSON = 0,
		TYPE_OTHERS_COMBINED = 1,
		TYPE_NOT_NA = 2,
		TYPE_COMBINED_MANULLY = 10
	};

	/*fields*/
	int naid;
	//ContactInfo* contactInfo;
	vector<string> names;
	//string alias;
	int contact_id;
	Type type;
	string email;
	string address;
	string phone;
	string fax;
	string homepage;
	string position;
	string affiliation;
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
	string interest;

	vector<int> interest_years;
	vector<string> interest_by_year;
	//statistic
	AuthorFeatures feature;

	int h_index;
	int citation_no;
	int pub_count;
	
	int org_id;
	//below fields wouldn't be add into the collection field meta data
	vector<int> pub_ids;

	vector<AuthorRelation> Coauthor;
public:
	Author(DocumentCollection* collection);
	~Author();
	void assign_statistics();
};

#define UPDATE_FEATURE 0x00000001

#define CONTACT_INFO_CHANGED 0x00000001
#define INTEREST_CHANGED 0x00000002
#define NAMES_CHANGED 0x00000004
#define TYPE_CHANGED 0x00000008

class AuthorAdapter
{
public:
	//AuthorAdapter();
	static AuthorAdapter& instance();
	~AuthorAdapter();

	void foreach(const std::function<void(Author*)>& callback);
	DocumentCollection* getDocCollection();
public:
	void update(Author* author, vector<const FieldInfo*>& updated_fields);
	int add(Author* author);
	void remove(vector<int>& naids);
	void reload(vector<int>& naids);
	void recalcufeature(int naid);
private:
	int get_change(vector<const FieldInfo*>& updated_fields);
	bool get_contact_info(Author& author, ContactInfo& contact_info);
public:
	void move(int from, int to, int pud_ids, int update=0);
	void merge(vector<int>& ids, const string& name, int contact_id, Author::Type type, int update=0);
	//void merge(int naid, string& name, int contact_id, int type, vector<Author2pub>& pubs);
private:
	Author* choose_contact(vector<Author*>& authors);
	void copy_contactinfo(Author* from, Author* to);
	vector<Author2pub> move_all_pubs(Author* from, Author* to);

	//merge all other names into the first one's alias
	void merge_names(vector<Author*>& authors);
	void attach_contact_info(Author* author, ContactInfo* info);
private:
	AuthorAdapter();
	void initCollection();
	DocumentCollection* collection;
	std::mutex update_mutex;

};

class GetContactID2NaidVectorFunc
{
public:
	GetContactID2NaidVectorFunc(DocumentCollection* _collection);
    GetContactID2NaidVectorFunc(const GetContactID2NaidVectorFunc&) = delete;
	virtual void operator()(NapersonItem* item);
	virtual ~GetContactID2NaidVectorFunc();
public:
	vector<int>* cont2naid;
	vector<Author*>* authors;
	DocumentCollection* collection;

};

class AttachContactInfo2AuthorFunc
{
public:
    AttachContactInfo2AuthorFunc(const AttachContactInfo2AuthorFunc&) = delete;
	virtual void operator()(ContactInfo* info);
	AttachContactInfo2AuthorFunc(vector<Author*>* _authors, vector<int>* _cid2naid);
private:
	vector<Author*>* authors;
	vector<int>* cid2naid;

};

class AttachInterest2AuthorFunc
{
public:
    AttachInterest2AuthorFunc(const AttachInterest2AuthorFunc&) = delete;
	virtual void operator()(InterestItem* item);
	AttachInterest2AuthorFunc(vector<Author*>* _authors);
private:
	vector<Author*>* authors;

};

class GetAuthorCollectionFunc
{
public:
	GetAuthorCollectionFunc(DocumentCollection* _collection);
    GetAuthorCollectionFunc(const GetAuthorCollectionFunc&) = delete;
	virtual void operator()(Author* author);
private:
	DocumentCollection* collection;

};

class AttachFeature2AuthorFunc
{
public:
	AttachFeature2AuthorFunc(vector<Author*>* _authors);
    AttachFeature2AuthorFunc(const AttachFeature2AuthorFunc&) = delete;
	virtual void operator()(sql::ResultSet* rs);
private:
	vector<Author*>* authors;

};

class AttachRelationFunc
{
public:
	AttachRelationFunc(vector<Author*>* _authors);
    AttachRelationFunc(const AttachRelationFunc&) = delete;
	virtual void operator()(sql::ResultSet* rs);
private:
	vector<Author*>* authors;
};

class AttachOrgFunc
{
public:
	AttachOrgFunc(vector<Author*>* _authors);
    AttachOrgFunc(const AttachOrgFunc&) = delete;
	virtual void operator()(sql::ResultSet* rs);
private:
	vector<Author*>* authors;
	OrganizationAdapter& adapter;
};
