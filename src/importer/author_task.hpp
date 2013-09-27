#pragma once
#include "BackstageTask.h"
#include "ContactInfoAdapter.h"
#include <string>
#include<vector>
using std::vector;
using std::string;

class MovePubTask: public BackstageTask
{
public:
	virtual void execute();
	MovePubTask(int from, int to, int pub_id);
private:
	int from, to, pub_id;
};

class Author2pub;

class MergeAuthorTask: public BackstageTask
{
public:
	virtual void execute();
	MergeAuthorTask(vector<int>&, int, const string&, int, int, vector<Author2pub>& pubs);
private:
	vector<int> from_ids;
	int to_id;
	string name;
	int contact_id;
	int type;
	vector<Author2pub> pubs;
};

class UpdateAuthorInterestTask: public BackstageTask
{
public:
	virtual void execute();
	UpdateAuthorInterestTask(int naid, const string& interest, int year);
private:
	int naid;
	string interest;
	int year;
};

class UpdateAuthorContactInfoTask: public BackstageTask
{
public:
	virtual void execute();
	UpdateAuthorContactInfoTask(const ContactInfo& model);
private:
	ContactInfo model;
};

class UpdateAuthor2ContactTask: public BackstageTask
{
public:
	virtual void execute();
	UpdateAuthor2ContactTask(int naid, const string& names, int contact_id, int type);
private:
	int naid;
	string names;
	int contact_id;
	int type;
};

class InsertContactInfoTask: public BackstageTask
{
public:
	virtual void execute();
	InsertContactInfoTask(int naid, ContactInfo& model);
private:
	int naid;
	ContactInfo model;
};
