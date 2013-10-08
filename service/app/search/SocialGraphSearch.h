#pragma once
#include <string>
#include <vector>
#include <set>
#include "AuthorRelation.h"

using std::string;
using std::vector;
using std::set;
using std::pair;

class SocialGraphSearch {
public:
	vector<int> person_id;
	vector<int> expand_id;
	vector<int> remove_id;
	int algo;
	
	SocialGraphSearch(vector<int> _person_id, vector<int> _expand_id, vector<int> _remove_id, int _algo);
	vector<AuthorRelation> getSocialNetwork();
	void getPersonSocailNetwork(int person_id, vector<pair<AuthorRelation, int> >& relation, vector<int>& coauthor, set<int>& target, int Limit);

	~SocialGraphSearch();
};
