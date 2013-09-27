#pragma once
#include "Logger.h"
#include<string>
using std::string;

namespace ConfService
{

	bool searchConferences(const string& input, string& output);
	bool getConferenceByName(const string& input, string& output);
	bool getConferenceById(const string& input, string& output);
	bool getConferenceByPrefix(const string& input, string& output);
};