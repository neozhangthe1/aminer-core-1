#include "Name2AuthorMap.h"
#include "AuthorAdapter.h"
#include "Timer.h"
#include "stringutils.h"
#include "MemGC.h"
#include "IndexResource.h"
#include "Atomic.h"
#include "search.h"
#include "SearchConfigure.h"

Name2AuthorMap& Name2AuthorMap::instance()
{
	static Name2AuthorMap _instance;
	return _instance;
}

Name2AuthorMap::Name2AuthorMap()
{
	//DO nothing here, wait for the IndexResource class to call the init function

	this->name2authorMap = new unordered_map<string, set<int>*>();
}

void Name2AuthorMap::init(DocumentCollection* authors)
{
	Timer timer;
	timer.start();

	Author* author = NULL;

	int size = authors->getDocCounts();
	
	for(int i=0;i<size;++i)
	{
		author = (Author*)authors->getDocumentByIndex(i);
		if(author)
		{
			for (string& name : author->names)
			{
				this->insert(name, author);
			}
		}
	}
	timer.stop();
	LOG(INFO) << boost::str(boost::format("ANN Load, Used %1% ms") % timer.getPassedMilliseconds());
}

set<int>* Name2AuthorMap::getAuthorsByAuthorName(string name)
{
	strutils::to_lower(name);
	strutils::trim(name);
	unordered_map<string, set<int>*>::iterator iter = name2authorMap->find(name);
	if(iter != name2authorMap->end())
	{
		return iter->second;
	}
	return NULL;
}

//authors[0]: the new author that is that is combination of the left authors
//authors[1~n-2]: authos that are merged into the new author
//authors[n-1]: the original pointer of the author who are merged into by others
void Name2AuthorMap::merge(vector<Author*>& authors)
{
	std::lock_guard<std::mutex> guard(this->update_mutex);
	unordered_map<string, set<int>*>* new_name2author_map = new unordered_map<string, set<int>*>(*this->name2authorMap);
	for (string str : authors[0]->names)
	{
		strutils::to_lower(str);
		set<int>* author_set=NULL;
		unordered_map<string, set<int>*>::iterator iter = new_name2author_map->find(str);
		if(iter != new_name2author_map->end())
		{
			author_set = iter->second;
			author_set = new set<int>(*author_set);
			for (Author* author : authors)
			{
				author_set->erase(author->naid);
			}
			author_set->insert(authors[0]->naid);
			{
				MemGC::instance().addGarbage(new DeleterImp<set<int>>(iter->second));
				iter->second = author_set;
			}
		}
	}
	assign_ptr<unordered_map<string, set<int>*>>(&(this->name2authorMap), new_name2author_map);
}

void Name2AuthorMap::insert(string name, Author* author)
{
	if(name.length()==0)
		return;
	
	strutils::to_lower(name);
	strutils::trim(name);
	//printf("inserting %s\n", name.c_str());
	set<int>* list=NULL;
	unordered_map<string, set<int>*>::iterator iter = name2authorMap->find(name);
	if(iter != name2authorMap->end())
	{
		list = iter->second;
	}
	else
	{
		list = new set<int>();
		name2authorMap->insert(make_pair(name, list));
	}
	list->insert(author->naid);
}

void Name2AuthorMap::remove(Author* author)
{
	if(!author)
		return;
	std::lock_guard<std::mutex> guard(this->update_mutex);
	unordered_map<string, set<int>*>* new_name2author_map = new unordered_map<string, set<int>*>(*this->name2authorMap);
	set<int>* author_set=NULL;
	unordered_map<string, set<int>*>::iterator iter; 
	for (string& name : author->names)
	{
		strutils::to_lower(name);
		strutils::trim(name);
		
		iter = new_name2author_map->find(name);
		if(iter!=new_name2author_map->end())
		{
			author_set = iter->second;
			author_set = new set<int>(*author_set);
			MemGC::instance().addGarbage(new DeleterImp<set<int>>(iter->second));
			author_set->erase(author->naid);
			if(author_set->empty())
			{
				delete author_set;
				new_name2author_map->erase(iter);
			}
			else
			{
				iter->second = author_set;
			}
		}
	}
	assign_ptr<unordered_map<string, set<int>*>>(&name2authorMap, new_name2author_map);
}

void Name2AuthorMap::add(Author* author)
{
	if(!author)
		return;
	std::lock_guard<std::mutex> guard(this->update_mutex);
	unordered_map<string, set<int>*>* new_name2author_map = new unordered_map<string, set<int>*>(*this->name2authorMap);
	set<int>* author_set=NULL;
	unordered_map<string, set<int>*>::iterator iter; 
	for (string name : author->names)
	{
		strutils::to_lower(name);
		strutils::trim(name);
		iter = new_name2author_map->find(name);
		if(iter!= new_name2author_map->end())
		{
			author_set = new set<int>(*iter->second);
			author_set->insert(author->naid);
			MemGC::instance().addGarbage(new DeleterImp<set<int>>(iter->second));
			iter->second = author_set;
		}
		else
		{
			author_set = new set<int>();
			author_set->insert(author->naid);
			new_name2author_map->insert(make_pair(name, author_set));
		}
	}
	assign_ptr<unordered_map<string, set<int>*>>(&name2authorMap, new_name2author_map);
}

bool Name2AuthorMap::searchAuthorsByName(const string& name, vector<int>& naids)
{
	IndexResource& resource = IndexResource::instance();
	DocumentCollection* authors = resource.authorCollection();

	IndexList* index = resource.authorIndex();
	Searcher searcher(index, new StandardQueryAnalyzer(index));
	searcher.SetSearchedFields(SearchConfigure::instance().authorDefSearchFields);
	QueryItemEnumerator* enumer = searcher.Search(name);

	if(!enumer)
		return false;
	
	naids.clear();
	QueryItem item;
	Author* author;
	while(enumer->next(item))
	{
		author = (Author*)authors->getDocumentByIndex(item.docId);
		if(author)
			naids.push_back(item.docId);
	}
	delete enumer;
	return !naids.empty();
}
