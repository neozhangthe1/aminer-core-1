#pragma once
#include <vector>
#include <string>
#include <set>
#include <mutex>
#include <unordered_map>
#include "Logger.h"
using std::string;
using std::vector;
using std::set;
using std::unordered_map;

class DocumentCollection;
class Author;

class Name2AuthorMap
{
public:
	static Name2AuthorMap& instance();
	set<int>* getAuthorsByAuthorName(string name);
	bool searchAuthorsByName(const string& name, vector<int>& naids);
	//This function is special for class IndexResource
	void init(DocumentCollection* authors);

	//Merge into the first one
	void merge(vector<Author*>& authors);

	//remove this author from map
	void remove(Author* author);

	void add(Author* author);
private:
	Name2AuthorMap();
	void insert(string name, Author* author);
	unordered_map<string, set<int>*>* volatile name2authorMap;

	std::mutex update_mutex;
};
