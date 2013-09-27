#pragma once
#include "index.h"
#include "Name2AuthorMap.h"
#include "Logger.h"

class IndexResource
{
public:
	static IndexResource& instance();

	IndexList* pubIndex();
	IndexList* authorIndex();

	DocumentCollection* orgCollection();
	DocumentCollection* pubCollection();
	DocumentCollection* authorCollection();

	//Default return fields of author
	vector<string>& authorDefRetFields();

	//Default return features of author
	vector<string>& authorDefRetFeatures();

	//Default return fields of publication
	vector<string>& pubDefRetFields();

	//Name to Authors Map
	Name2AuthorMap& name2AuthorMap();
private:
	IndexResource();

	//Author.pubs and conf.pub would be setted here
	void assignPub2ConfAndAuthor();

	//Assign topic_id foreach publication
	void confirmPubTopic(int part_id, int part_count);
private:
	IndexList* _pubIndex;
	IndexList* _authorIndex;

	DocumentCollection* _pubCollection;
	DocumentCollection* _authorCollection;
	DocumentCollection* _orgCollection;

	//Map
	Name2AuthorMap& _name2AuthorMap;


};
