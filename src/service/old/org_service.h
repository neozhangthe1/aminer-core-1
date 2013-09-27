#pragma once
#include "interface.pb.h"
#include "OrganizationAdapter.h"
#include "InterfaceUtils.h"
#include "Logger.h"

namespace OrgService
{


	bool getOrgnizationById(const string& input, string& output);
	bool getOrganizationByPrefix(const string& input, string& output);
	bool getTopHindexAuthors(const string& input, string& output);
	bool getTopicTrend(const string& input, string & output);
	bool getHotTopics(const string& input,  string& output);
}