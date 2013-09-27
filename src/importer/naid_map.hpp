#pragma once
#include <unordered_map>
#include <vector>
#include <boost/thread.hpp>

using std::vector;
using std::unordered_map;

class NaidMap
{
public:
	static NaidMap& instance();
	void add(vector<int>& froms, int to);
	int get(int);
private:
	NaidMap();
	unordered_map<int,int> id_map;
    boost::shared_mutex mutex_;
};
