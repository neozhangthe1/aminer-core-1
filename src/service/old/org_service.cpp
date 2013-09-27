#include "OrgService.h"
#include "AuthorAdapter.h"
#include "stringutils.h"
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

namespace {
	inline bool compare_org_name(const Organization* left, const Organization* right)
	{
		return left->name<right->name;
	}
}

namespace OrgService
{
	/*
	*Input: mashaler::IntQueryParam
	*Ouput: mashaler::OrganizationResult
	*/
	bool getOrgnizationById(const string& input, string& output)
	{
		mashaler::IntQueryParams params;
		params.ParseFromString(input);

		DocumentCollection* organizations = OrganizationAdapter::instance().collection;

		vector<int> ids;
		boost::range::copy(params.ids(), std::back_inserter(ids));
		LOG(INFO) << boost::str(boost::format("[id] %s") % strutils::vectorToString(ids));

		mashaler::OrganizationResult result;
		Document* doc;
		for(int i=0; i < params.ids_size(); i++)
		{
			int onePubID = params.ids(i);

			doc = organizations->getDocumentByIndex(onePubID);
			if(doc)
			{
				ProtoConverter::convert(organizations, *doc, *result.add_organizations(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
	}

	/*
	*Get a list of organizations using the given prefix with alphabetical order
	*	case '#': all
	*	case '!': special character
	*	default: prefix
	*
	*Input: mashaler::StringQueryParam
	*Output: mashaler::OrganizationResult
	*
	*/
	bool getOrganizationByPrefix(const string& input, string& output)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		LOG(INFO) << boost::str(boost::format("[prefix] %s") % params.query());

		Organization* org = NULL;
		vector<Organization*> orgs;

		OrganizationAdapter::instance().getOrgWithPrefix(orgs, params.query());
		mashaler::OrganizationResult result;

		if(!orgs.empty())
		{
			DocumentCollection* organizations = OrganizationAdapter::instance().collection;
			sort(orgs.begin(), orgs.end(), compare_org_name);
			for (auto org : orgs)
			{
				ProtoConverter::convert(organizations, *org, *result.add_organizations(), params.returned_fields());
			}
		}
		return result.SerializeToString(&output);
	}

	//Get authos with top hindex of the given organization
	//
	//Input: mashaler::IntQueryParam
	//Output: mashaler::AuthorResult
	bool getTopHindexAuthors(const string& input, string& output)
	{
		mashaler::IntQueryParams param;
		mashaler::AuthorResult result;
		OrganizationAdapter& adapter = OrganizationAdapter::instance();

		vector<Author*> authors;
		param.ParseFromString(input);
		if(param.ids_size()==0)
			goto fail;
		if(!adapter.collection->getDocumentByIndex(param.ids(0)))
			goto fail;
		if(!adapter.getTopHindexAuthors(param.ids(0), param.count(), authors))
			goto fail;

		LOG(INFO) << boost::str(boost::format("[top_author] %d") % param.ids(0));

		if(!authors.empty())
		{
			DocumentCollection* author_col = AuthorAdapter::instance().getDocCollection();
			for (Author* author : authors)
			{
				ProtoConverter::convertAuthor(author_col, *author, *result.add_authors(), param.returned_fields());
			}
		}
success:
		return result.SerializeToString(&output);
fail:
		return false;
	}

	/*
	*	Get topic trend of given organization
	*	Input: int
	*	Output: mashaler::TopicTrend
	*/
	bool getTopicTrend(const string& input, string& output)
	{
		LOG(INFO) << boost::str(boost::format("[topic_trend] %s") % input);

		int org_id = atoi(input.c_str());
		mashaler::TrendData result;
		OrganizationAdapter&adapter = OrganizationAdapter::instance();
		vector<int> topics;

		if(!adapter.collection->getDocumentByIndex(org_id))
            return false;

		if(!adapter.getHotTopic(org_id, -1, topics))
            return false;

		int min = INT_MAX;
		for (int topic : topics)
		{
			result.add_topic_ids(topic);
			int start_year;
			vector<int> pub_counts;
			adapter.getTopicTrend(org_id, topic, start_year, pub_counts);
			result.add_start_years(start_year);
			mashaler::Distribution* trend = result.add_trends();
			for (int pub_count : pub_counts)
			{
				trend->add_values(pub_count);
			}
		}

		return result.SerializeToString(&output);
	}

	/*
	*	Get hot topics of given organization
	*	Input: int
	*	Output: mashaler::TopicTrend
	*/
	bool getHotTopics(const string& input,  string& output)
	{
		LOG(INFO) << boost::str(boost::format("[hot_topic] %s") % input);

		int org_id = atoi(input.c_str());
		mashaler::Distribution result;
		OrganizationAdapter&adapter = OrganizationAdapter::instance();
		vector<int> topics;

		if(!adapter.collection->getDocumentByIndex(org_id))
			goto fail;

		if(!adapter.getHotTopic(org_id, -1, topics))
			goto fail;

		for (int topic : topics)
		{
			result.add_values(topic);
		}

success:
		return result.SerializeToString(&output);
fail:
		return false;
	}
}
