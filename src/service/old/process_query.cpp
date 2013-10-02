#include "process_query.hpp"

namespace ProcessQuery
{
	bool get_param(string& query, const string& param_name, string& param_value)
	{
		size_t pos = query.find(param_name);
		if(pos==string::npos)
			return false;
		size_t colon_pos = query.find(':', pos+1);
		if(colon_pos == string::npos || query.length()<=(colon_pos+2) || query[colon_pos+1]!='[')
			return false;
		size_t brack_pos = query.find(']', colon_pos+2);
		if(brack_pos == string::npos)
			return false;
		param_value.assign(query.begin()+colon_pos+2, query.begin()+brack_pos);
		query.erase(pos, brack_pos-pos+1);
		return true;
	}

	bool get_year(string& query, Condition& condition)
	{
		size_t pos = query.find("year");
		if(pos==string::npos || query.length()<pos+6)
			return false;

		condition.field="year";
		condition.op.clear();
		string::iterator iter = query.begin()+pos+4;
		while(iter!=query.end() && (*iter=='<' || *iter=='>' || *iter=='=' || *iter=='!'))
		{
			condition.op+=*iter;
			++iter;
		}
		if(condition.op.empty())
			return false;
		int count = 0;
		while(iter!=query.end() && *iter<='9' && *iter>='0')
		{
			++count;
			++iter;
		}
		if(!count)
			return false;
		condition.value.assign(iter-count,iter);
		query.erase(query.begin()+pos, iter);
		return true;
	}
}