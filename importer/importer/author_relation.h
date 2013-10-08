#pragma once
#include <string>

using std::string;

class AuthorRelation {
public:
	int pid1;
	int pid2;
	double similarity;
	string rel_type;
	AuthorRelation(int _pid1, int _pid2,double _similarity,string _rel_type);
	AuthorRelation();
	~AuthorRelation();
};
