#include "search_filter.hpp"

SearchCondition::SearchCondition(DocumentCollection* _collection, const FieldInfo* _field)
	:collection(_collection),
	field(_field)
{
}

SearchFilter::SearchFilter(QueryItemEnumerator* _input, DocumentCollection* _collection)
	:input(_input),
	collection(_collection)
{
}

SearchFilter::~SearchFilter()
{
	for(vector<SearchCondition*>::iterator iter = conditions.begin(); iter!= conditions.end(); ++iter)
	{
		delete *iter;
	}
	delete input;
}

bool SearchFilter::next(QueryItem& item)
{
	while(input->next(item))
	{
		if(check(item))
			return true;
	}
	return false;
}

int SearchFilter::size()
{
	// TODO: may be I need to count the size here or estimate
	return input->size();
}

bool SearchFilter::addCondition(Condition& con)
{
	SearchCondition* condition = NULL;
	const FieldInfo* field = collection->getFieldByName(con.field);
	if(!field)
		return false;

	bool flag = false;
	if(con.op == ">" || con.op == "<" || con.op == "=" || con.op == ">=" || con.op == "<=" || con.op=="!=")
	{
		flag = true;
	}
	else
	{
		return false;
	}

	switch(field->type)
	{
	case FieldType::Int:
		{
			int value = atoi(con.value.c_str());
			condition = new SearchConditionImp<int>(collection, field, value, con.op);
		}
		break;

	case FieldType::Double:
		{	
			double value = atof(con.value.c_str());
			condition = new SearchConditionImp<double>(collection, field, value, con.op);
		}
		break;

	case FieldType::String:
		{
			condition = new SearchConditionImp<string>(collection, field, con.value, con.op);
		}
		break;

	default:
		flag = false;
	}
	this->conditions.push_back(condition);
	return flag;
}

bool SearchFilter::check(QueryItem& item)
{
	for(vector<SearchCondition*>::iterator iter = conditions.begin(); iter!= conditions.end(); ++iter)
	{
		if(!(*iter)->check(item))
			return false;
	}
	return true;
}