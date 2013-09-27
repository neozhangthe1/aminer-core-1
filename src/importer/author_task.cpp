#include "AuthorTask.h"
#include "Author2pubDao.h"
#include "Author2ContactDao.h"
#include "AuthorUtils.h"
#include "MergedAuthorDao.h"
#include "AuthorInterestDao.h"

MovePubTask::MovePubTask(int _from, int _to, int _pub_id)
	:from(_from),
	to(_to),
	pub_id(_pub_id)
{
}

void MovePubTask::execute()
{
	Author2pubDao dao;
	dao.move(from, to, pub_id);
	//printf("move %d %d %d\n", from, to, pub_id);
}

MergeAuthorTask::MergeAuthorTask(vector<int>& _from_ids, int _to_id, const string& _name, int _contact_id, int _type, vector<Author2pub>& _pubs)
	:from_ids(_from_ids),
	to_id(_to_id),
	name(_name),
	contact_id(_contact_id),
	type(_type),
	pubs(_pubs)
{
}

void MergeAuthorTask::execute()
{
	Author2ContactDao a2c;
	a2c.updateItem(to_id, name, contact_id, type);


	Author2pubDao a2p;
	a2p.move(to_id, pubs);

	//Remove others
	a2c.removeItem(from_ids);

	MergedAuthorDao mg_dao;
	mg_dao.add(from_ids, to_id);
}


UpdateAuthorInterestTask::UpdateAuthorInterestTask(int _naid, const string& _interest, int _year)
	:naid(_naid),
	interest(_interest),
	year(_year)
{
}

void UpdateAuthorInterestTask::execute()
{
	AuthorInterestDao dao;
	dao.updare_or_insert(naid, interest, year);
}

UpdateAuthorContactInfoTask::UpdateAuthorContactInfoTask(const ContactInfo& _model)
	:model(_model)
{
}

void UpdateAuthorContactInfoTask::execute()
{
	ContactInfoAdapter& adapter = ContactInfoAdapter::instance();
	adapter.update(model);
}

UpdateAuthor2ContactTask::UpdateAuthor2ContactTask(int _naid, const string& _names, int _contact_id, int _type)
	:naid(_naid),
	names(_names),
	contact_id(_contact_id),
	type(_type)
{
}

void UpdateAuthor2ContactTask::execute()
{
	Author2ContactDao dao;
	dao.updateItem(naid, names, contact_id, type);
}

InsertContactInfoTask::InsertContactInfoTask(int _naid, ContactInfo& _model)
	:naid(_naid),
	model(_model)
{
}

void InsertContactInfoTask::execute()
{
	DocumentCollection* authors = AuthorAdapter::instance().getDocCollection();
	Author* author = (Author*)authors->getDocumentByIndex(naid);
	if(author)
	{
		Author* new_author = new Author(*author);
		new_author->contact_id = ContactInfoAdapter::instance().insert(model);
		authors->assign(naid, new_author);
		BackstageTasks::instance().addTask(new UpdateAuthor2ContactTask(naid, string(), new_author->contact_id, -1));
	}
}
