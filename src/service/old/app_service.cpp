#include "InterfaceUtils.h"
#include "AppService.h"
#include "interface.pb.h"
#include "CollaboratorRecommend.h"
#include "ProcessQuery.h"
#include "IndexResource.h"

namespace AppService
{
	bool CollaboratorRecommend(const string& input, string& output)
	{
		mashaler::StringQueryParams params;
		params.ParseFromString(input);

		string query = params.query();
		string naid;
		if(ProcessQuery::get_param(query, "naid", naid))
		{
			CollaboratorRecommender recommender;
			int src_naid = atoi(naid.c_str());
			vector<int>&& ids = recommender.recommend(src_naid, query);
			Author* author;
			DocumentCollection* authors = IndexResource::instance().authorCollection();
			mashaler::AuthorResult result;
			for (int id : ids)
			{
				author = (Author*)authors->getDocumentByIndex(id);
				if(author)
				{
					ProtoConverter::convertAuthor(authors , *author, *result.add_authors(), params.returned_fields());
				}
			}
			return result.SerializeToString(&output);
		}
		else 
			return false;
	}
}
