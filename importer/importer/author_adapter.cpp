#include <cstddef>
#include <cppconn/prepared_statement.h>
#include <boost/algorithm/string.hpp>

#include "db/Author2ContactDao.h"
#include "db/AuthorFeatureDao.h"
#include "db/AuthorInterestDao.h"
#include "db/AuthorRelationDao.h"

#include "document.h"
#include "stringutils.h"
#include "interface.pb.h"

#include "author_task.h"
#include "author_relation.h"
#include "name2author_map.h"
#include "naid_map.h"

#include "author_adapter.h"
#include "contact_adapter.h"
#include "pub_adapter.h"

#include "MemGC.h"


Author::Author(DocumentCollection* _collection)
	:Document(_collection),
	naid(-1),
	contact_id(-1),
	type(Author::TYPE_REALPERSON),
	org_id(-1)
{
}

Author::~Author()
{
}

void Author::assign_statistics()
{
	if(this->feature.scores.empty())
		return;
	double* scores = this->feature.scores[0];
	this->h_index = (int)scores[AuthorFeatures::HINDEX];
	this->citation_no = (int)scores[AuthorFeatures::CITATIONNO];
	this->pub_count = (int)scores[AuthorFeatures::PUBCOUNT];
}

AuthorAdapter& AuthorAdapter::instance()
{
	static AuthorAdapter _instance;
	return _instance;
}

AuthorAdapter::AuthorAdapter()
{

	initCollection();
}

AuthorAdapter::~AuthorAdapter()
{
#ifdef SERVERDB
	printf("~AuthorAdapter()\n");
#endif
}

void AuthorAdapter::foreach(const function<void(Author*)>& callback)
{
	GetContactID2NaidVectorFunc c2aFunc(collection);
	Author2ContactDao a2cDao;
	a2cDao.foreach(ref(c2aFunc));

	vector<int>* c2a = c2aFunc.cont2naid;
	vector<Author*>* authors = c2aFunc.authors;

	AttachContactInfo2AuthorFunc att2aFunc(authors, c2a);
	ContactInfoAdapter& conDao = ContactInfoAdapter::instance();
	conDao.foreach(ref(att2aFunc));

	AttachInterest2AuthorFunc attIntFunc(authors);
	AuthorInterestDao interestDao;
	interestDao.foreach(ref(attIntFunc));

	AttachFeature2AuthorFunc attFeaFunc(authors);
	AuthorFeatureDao feaDao;
	feaDao.foreach(ref(attFeaFunc));

	AttachRelationFunc attRelaFunc(authors);
	AuthorRelationDao RelationDao;
	RelationDao.foreach(ref(attRelaFunc));

	AttachOrgFunc attOrgFunc(authors);
	Author2OrgDao orgDao;
	orgDao.foreach(ref(attOrgFunc));

	for(vector<Author*>::iterator iter = authors->begin(); iter!=authors->end();++iter)
	{
		if(*iter)//unless the pointer is not null;
		{
			//assign h-index, pub_count etc.
			(*iter)->assign_statistics();
			callback(*iter);		
		}
	}
}

DocumentCollection* AuthorAdapter::getDocCollection()
{
	return collection;
}

void AuthorAdapter::initCollection()
{
	vector<FieldInfo*> fields;
	FieldInfo* field;

	mashaler::Author author;
	const google::protobuf::Descriptor* descriptor = author.GetDescriptor();

	field = new FieldInfo();
	field->name = "naid";
	field->offset = offsetof(Author, naid);
	field->type = FieldType::Int;
	field->protoField = descriptor->FindFieldByName("naid");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "names";
	field->isIndexed = true;
	field->weight = 1.0;
	field->average_field_length = 2.0;
	field->offset = offsetof(Author, names);
	field->type = FieldType::StringVector;
	field->protoField = descriptor->FindFieldByName("names");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "type";
	field->offset = offsetof(Author, type);
	field->type = FieldType::Int;
	field->protoField = descriptor->FindFieldByName("type");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "contact_id";
	field->offset = offsetof(Author, contact_id);
	field->type = FieldType::Int;
	field->protoField = descriptor->FindFieldByName("contact_id");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "email";
	field->offset = offsetof(Author, email);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("email");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "address";
	field->offset = offsetof(Author, address);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("address");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "phone";
	field->offset = offsetof(Author, phone);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("phone");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "fax";
	field->offset = offsetof(Author, fax);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("fax");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "homepage";
	field->offset = offsetof(Author, homepage);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("homepage");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "position";
	field->offset = offsetof(Author, position);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("position");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "affiliation";
	field->offset = offsetof(Author, affiliation);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("affiliation");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "imgurl";
	field->offset = offsetof(Author, imgurl);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("imgurl");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "imgname";
	field->offset = offsetof(Author, imgname);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("imgname");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "imgsrc";
	field->offset = offsetof(Author, imgsrc);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("imgsrc");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "phduniv";
	field->offset = offsetof(Author, phduniv);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("phduniv");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "phdmajor";
	field->offset = offsetof(Author, phdmajor);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("phdmajor");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "phddate";
	field->offset = offsetof(Author, phddate);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("phddate");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "msuniv";
	field->offset = offsetof(Author, msuniv);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("msuniv");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "msmajor";
	field->offset = offsetof(Author, msmajor);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("msmajor");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "msdate";
	field->offset = offsetof(Author, msdate);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("msdate");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "bsuniv";
	field->offset = offsetof(Author, bsuniv);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("bsuniv");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "bsmajor";
	field->offset = offsetof(Author, bsmajor);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("bsmajor");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "bsdate";
	field->offset = offsetof(Author, bsdate);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("bsdate");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "bio";
	field->offset = offsetof(Author, bio);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("bio");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "interest";
	field->offset = offsetof(Author, interest);
	field->type = FieldType::String;
	field->protoField = descriptor->FindFieldByName("interest");
	assert(field->protoField!=NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "interest_years";
	field->offset = offsetof(Author, interest_years);
	field->type = FieldType::IntVector;
	field->protoField = descriptor->FindFieldByName("interest_years");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "interest_by_year";
	field->offset = offsetof(Author, interest_by_year);
	field->type = FieldType::StringVector;
	field->protoField = descriptor->FindFieldByName("interest_by_year");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "feature";
	field->offset = offsetof(Author, feature);
	field->type = FieldType::Other;
	field->protoField = descriptor->FindFieldByName("feature");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "h_index";
	field->type = Int;
	field->offset = offsetof(Author,h_index);
	field->protoField = descriptor->FindFieldByName("h_index");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "citation_no";
	field->type = Int;
	field->offset = offsetof(Author,citation_no);
	field->protoField = descriptor->FindFieldByName("citation_no");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "pub_count";
	field->type = Int;
	field->offset = offsetof(Author, pub_count);
	field->protoField = descriptor->FindFieldByName("pub_count");
	assert(field->protoField != NULL);
	fields.push_back(field);

	field = new FieldInfo();
	field->name = "org_id";
	field->type = Int;
	field->offset = offsetof(Author, org_id);
	field->protoField = descriptor->FindFieldByName("org_id");
	assert(field->protoField != NULL);
	fields.push_back(field);

	fields.push_back(field);
	collection = new DocumentCollection(new FieldsInfo(fields));

	Author2ContactDao a2cDao;
	collection->resize(a2cDao.getMaxId()+1);

	GetAuthorCollectionFunc func(collection);
	this->foreach(ref(func));
}

GetContactID2NaidVectorFunc::GetContactID2NaidVectorFunc(DocumentCollection* _collection)
{
	ContactInfoAdapter& cia = ContactInfoAdapter::instance();
	cont2naid = new vector<int>(cia.getMaxId()+1,-1);
	authors = new vector<Author*>(_collection->getDocCounts(), NULL);
	collection = _collection;

}

void GetContactID2NaidVectorFunc::operator()(NapersonItem* item)
{
	Author* author = new Author(collection);
	author->naid = item->naid;
	author->names = item->names;
	author->contact_id = item->contactid;
	author->type = static_cast<Author::Type>(item->type);
	(*authors)[item->naid] = author;
	if(item->contactid !=-1)
	{
		(*cont2naid)[item->contactid] = item->naid;
	}
	delete item;
}

GetContactID2NaidVectorFunc::~GetContactID2NaidVectorFunc()
{
	delete cont2naid;
	delete authors;
}

void AttachContactInfo2AuthorFunc::operator()(ContactInfo* info)
{
	int naid = this->cid2naid->at(info->contactId);

	if(naid==-1)
		return;

	Author* author = authors->at(naid);

	author->naid = naid;
	//author->name = info->name;
	author->email = info->email;
	author->address = info->address;
	author->phone = info->phone;
	author->fax = info->fax;
	author->homepage = info->homepage;
	author->position = info->position;
	author->affiliation = info->affiliation;
	author->imgurl = info->imgurl;
	author->imgname = info->imgname;
	author->imgsrc = info->imgsrc;
	author->phduniv = info->phduniv;
	author->phdmajor = info->phdmajor;
	author->phddate = info->phddate;
	author->msuniv = info->msuniv;
	author->msmajor = info->msmajor;
	author->msdate = info->msdate;
	author->bsuniv = info->bsuniv;
	author->bsmajor = info->bsmajor;
	author->bsdate = info->bsdate;
	author->bio = info->bio;
	delete info;
}

AttachContactInfo2AuthorFunc::AttachContactInfo2AuthorFunc(vector<Author*>* _authors, vector<int>* _cid2naid)
{
	this->authors = _authors;
	this->cid2naid = _cid2naid;

	LOG(INFO) << "Start";
}

AttachInterest2AuthorFunc::AttachInterest2AuthorFunc(vector<Author*>* _authors)
{
	authors = _authors;

}

void AttachInterest2AuthorFunc::operator()(InterestItem* item)
{
	if(item->id%100000==0)
		LOG(INFO) << boost::str(boost::format("Loading interest %1%\tAuthor %2%") % item->id % item->naid);
	Author* author;
	if(item->naid >= authors->size())
		goto clean_up;
	author = authors->at(item->naid);
	if(author)
	{
		if(item->year==1 || item->year==-1)
		{
			if(author->interest.empty() || item->year==-1)
			{
				author->interest.assign(item->interest);
			}
		}
		else
		{
			if(item->year>0)
			{
				author->interest_by_year.push_back(item->interest);
				author->interest_years.push_back(item->year);
			}
			else
			{
				item->year *= -1;
				for(int index = 0;index<author->interest_years.size();++index)
				{
					if(author->interest_years[index] == item->year)
					{
						author->interest_by_year[index] = item->interest;
					}
				}
			}
		}
	}
clean_up:
	delete item;
}

GetAuthorCollectionFunc::GetAuthorCollectionFunc(DocumentCollection* _collection)
{
	this->collection = _collection;
}

void GetAuthorCollectionFunc::operator()(Author* author)
{
	collection->assign(author->naid, author);
}

AttachFeature2AuthorFunc::AttachFeature2AuthorFunc(vector<Author*>* _authors)
{
	authors = _authors;

}

void AttachFeature2AuthorFunc::operator()(sql::ResultSet* rs)
{
	int person_id = rs->getInt(1);
	if(person_id>=authors->size())
		return;
	Author* author = authors->at(person_id);
	if(!author)
	{
		return;
	}
	assert(author!=NULL);

	if(person_id%10000==0)
		LOG(INFO) << boost::str(boost::format("AttachFeature2AuthorFunc: Person%1%") % person_id);
	author->feature.add(rs);
}

AttachRelationFunc::AttachRelationFunc(vector<Author*>* _authors)
{
	authors = _authors;
}

void AttachRelationFunc::operator()(sql::ResultSet* rs)
{
	int person_id1 = rs->getInt(2);
	int person_id2 = rs->getInt(3);
	double similarity = rs->getDouble(4);
	string rel_type = rs->getString(5);

	if (person_id1 < 0 || person_id2 < 0 || person_id1 >= authors->size() || person_id2 >= authors->size())
	{
		return;
	}

	Author* author = authors->at(person_id1);
	if(!author)
	{
		return;
	}
	assert(author!=NULL);

	AuthorRelation relation(person_id1, person_id2, similarity, rel_type);
	author->Coauthor.push_back(relation);

	int tmp = author->Coauthor.size() - 1;
	while (tmp > 0 && author->Coauthor[tmp].similarity > author->Coauthor[tmp - 1].similarity)
	{
		AuthorRelation temp = author->Coauthor[tmp];
		author->Coauthor[tmp] = author->Coauthor[tmp - 1];
		author->Coauthor[tmp - 1] = temp;
		tmp--;
	}

	author = authors->at(person_id2);
	if(!author)
	{
		return;
	}
	assert(author!=NULL);

	relation.pid1 = person_id2;
	relation.pid2 = person_id1;

	author->Coauthor.push_back(relation);

	tmp = author->Coauthor.size() - 1;
	while (tmp > 0 && author->Coauthor[tmp].similarity > author->Coauthor[tmp - 1].similarity)
	{
		AuthorRelation temp = author->Coauthor[tmp];
		author->Coauthor[tmp] = author->Coauthor[tmp - 1];
		author->Coauthor[tmp - 1] = temp;
		tmp--;
	}
}

AttachOrgFunc::AttachOrgFunc(vector<Author*>* _authors)
	:authors(_authors),
	adapter(OrganizationAdapter::instance())
{

}

void AttachOrgFunc::operator()(sql::ResultSet* rs)
{
	int naid = rs->getInt(2);
	if(naid>authors->size())
		return;
	Author* author = this->authors->at(naid);
	if(!author)
		return;
	if(naid%10000==0)
		printf("Attach Organization for Naid = %d\n", naid);
	//author->org_id = adapter.get_id(rs);
	int oid = rs->getInt(3);
	if(oid<=0)
		return;
	author->org_id = oid;
}
