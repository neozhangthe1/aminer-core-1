#pragma once
#include <functional>
//#include "search.h"

class Condition
{
public:
	string field;
	string op;
	string value;
};

class SearchCondition
{
public:
	SearchCondition(DocumentCollection* _collection, const FieldInfo* field);
	virtual bool check(QueryItem& item)=0;
	virtual ~SearchCondition(){}
protected:
	DocumentCollection* collection;
	const FieldInfo* field;
};

template<typename T>
class SearchConditionImp: public SearchCondition
{
public:
	SearchConditionImp(DocumentCollection* collection, const FieldInfo* field, T& _value, const string& op)
		:SearchCondition(collection, field),
		value(_value)
	{
		if (op==">")
		{
            compare = std::greater<T>();
		}
		else if (op == "=")
		{
			compare = std::equal_to<T>();
		}
		else if (op == "<")
		{
			compare = std::less<T>();
		}
		else if (op == ">=")
		{
			compare = std::greater_equal<T>();
		}
		else if (op == "<=")
		{
			compare = std::less_equal<T>();
		}
		else if (op == "!=")
		{
			compare = std::not_equal_to<T>();
		}
	}

	virtual bool check(QueryItem& item)
	{
		Document* doc = collection->getDocumentByIndex(item.docId);
		if(!doc)
			return false;
		return compare(*((T*)doc->getFieldPtr(field)),  value);
	}

private:
	T value;
    std::function<bool(const T&, const T&)> compare;
};

class SearchFilter: public QueryItemEnumerator
{
public:
	/*
	*The input here would be deleted after the search
	*/
	SearchFilter(QueryItemEnumerator* _input, DocumentCollection* _collection);
	virtual ~SearchFilter();
	virtual bool next(QueryItem& item);
	virtual int size();
	bool addCondition(Condition& con);
private:
	bool check(QueryItem& item);
private:
	vector<SearchCondition*> conditions;
	QueryItemEnumerator* input;
	DocumentCollection* collection;
};
