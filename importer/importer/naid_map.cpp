#include "NaidMap.h"
#include "MergedAuthorDao.h"

NaidMap& NaidMap::instance()
{
	static NaidMap _instance;
	return _instance;
}

NaidMap::NaidMap()
{
	MergedAuthorDao dao;
	dao.load_into(this->id_map);
}

int NaidMap::get(int naid)
{
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
	unordered_map<int,int>::iterator iter = this->id_map.find(naid);
	if(iter==this->id_map.end())
		return naid;
	return iter->second;
}

void NaidMap::add(vector<int>& froms, int to)
{
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    for (int from: froms)
	{
		this->id_map[from] = to;
	}
}
