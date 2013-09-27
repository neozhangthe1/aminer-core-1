#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Logger.h"
#include "ConnectionPool.hpp"

class Author2pub
{
public:
	Author2pub(int _id, int _aid, int _pid, int _position);

public:
	int id;
	int aid;
	int pid;
	int position;
};

class Author2pubDao
{
public:
	const string TABLE_NAME;
	const string ABBR_NAME;
	const string FIELD_LIST;

	Author2pubDao(void);

	~Author2pubDao(void);

	vector<Author2pub*> fetchFrom(int id, int limit);
	vector<int> getAuthorPubs(int naid);
	vector<int> gerPubAuthors(int pub_id);
	int count();

	int batchInsert(const vector<Author2pub>& a2ps);

	void walk(const std::function<void(Author2pub*)>& callback);

	void move(int from, int to, int pub_id);
	void move(int to, vector<Author2pub>& pubs);
public:
	void remove_author(int naid);
	void remove_pub(int pub_id);
private:
	Author2pub* constructModel(ResultSet* rs);

private:

	string sql_fetchFrom;
	string sql_batchInsert;

};

class GetPub2AuthorMapFunc
{
	typedef unordered_map<int, vector<Author2pub*>> P2AMap;
public:
	GetPub2AuthorMapFunc();
    GetPub2AuthorMapFunc(const GetPub2AuthorMapFunc& that) = delete;

	virtual ~GetPub2AuthorMapFunc();

	virtual void operator()(Author2pub* model);

	vector<Author2pub*> getAuthorPubByPID(int pID);

	vector<int> getAuthorIDsByPID(int pID);
private:
	P2AMap* p2a;
	unordered_map<int,int> max_position;

};
