#include "SocialGraphSearch.h"
#include "AuthorAdapter.h"
#include "AuthorRelation.h"
#include <algorithm>
#include <vector>

using namespace std;

SocialGraphSearch::SocialGraphSearch(vector<int> _person_id, vector<int> _expand_id, vector<int> _remove_id, int _algo)
{
	person_id = _person_id;
	expand_id = _expand_id;
	remove_id = _remove_id;
	algo = _algo;
}

void SocialGraphSearch::getPersonSocailNetwork(int person_id, vector<pair<AuthorRelation, int> >& relation, vector<int>& coauthor, set<int>& target, int Limit)
{
	int roar = 0;
	coauthor.push_back(person_id);
	relation.push_back(make_pair(AuthorRelation(), -1));
	for (int i = 0; i < Limit; i++) {
		int num = coauthor.size();
		for (int j = roar; j < num; j++)
		{
			Author* guy = (Author*) AuthorAdapter::instance().getDocCollection()->getDocumentByIndex(coauthor[j]);
			if (guy == NULL)  
				continue;

			for (int k = 0; k < guy->Coauthor.size(); k++) {
				coauthor.push_back(guy->Coauthor[k].pid2);
				relation.push_back(make_pair(guy->Coauthor[k], j));
			}
			target.insert(coauthor[j]);
		}
		roar = num;
	}
}

vector<AuthorRelation> SocialGraphSearch::getSocialNetwork()
{
	vector<AuthorRelation> ret;
	vector<pair<AuthorRelation, int> > *relations = new vector<pair<AuthorRelation, int> >[this->person_id.size()];
	vector<int> *coauthors = new vector<int>[this->person_id.size()];

	int Limit = 0;
	set<int> target;
	if (person_id.size() == 1)
		Limit = 1;
	else 
		Limit = 3;
	for (int i = 0; i < person_id.size(); i++) {
		getPersonSocailNetwork(person_id[i], relations[i], coauthors[i], target, Limit);
	}
	int count = 0;
	vector<vector<AuthorRelation> > paths;
	if (algo == 1 && person_id.size() != 1)
	{
		for (int l : target) {
			for (int i = 0; i < person_id.size(); i++) {
				if (find(coauthors[i].begin(), coauthors[i].end(), l) != coauthors[i].end()) {
					for (int j = i + 1; j < person_id.size(); j++) 
						if (find(coauthors[j].begin(), coauthors[j].end(), l) != coauthors[j].end()) {
							vector<int>::iterator iter = find(coauthors[i].begin(), coauthors[i].end(), l);
							int k = iter-coauthors[i].begin();
							vector<AuthorRelation> path;
							path.clear();
							while (relations[i][k].second != -1) {
								path.push_back(relations[i][k].first);
								ret.push_back(relations[i][k].first);
								k = relations[i][k].second;
							}

							iter = find(coauthors[j].begin(), coauthors[j].end(), l);
							k = iter-coauthors[j].begin();
							while (relations[j][k].second != -1) {
								path.push_back(relations[j][k].first);
								ret.push_back(relations[j][k].first);
								k = relations[j][k].second;
							}
							paths.push_back(path);
							count++;
							if (count > 9)
								return ret;
						}
				}
			}
		}
	}

	if (algo == 1 && person_id.size() == 1)
	{
		for (int i = 0; i < min(relations[0].size(), (size_t) 15); i++) 
		{
			ret.push_back(relations[0][i].first);
		}
	}

	if (algo == 2)
	{
		for (int i = 0; i < relations[0].size(); i++) 
		{
			ret.push_back(relations[0][i].first);
		}
	}
	delete[] relations;
	delete[] coauthors;

	return ret;
}

SocialGraphSearch::~SocialGraphSearch()
{
}
