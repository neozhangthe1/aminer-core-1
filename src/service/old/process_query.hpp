#pragma once
//#include "SearchFilter.h"
#include <string>
using std::string;

namespace ProcessQuery
{
	bool get_param(string& query, const string& param_name, string& param_value);

	bool get_year(string& query, Condition& condition);
}