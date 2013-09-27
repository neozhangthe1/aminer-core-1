#include "AuthorRelation.h"

AuthorRelation::AuthorRelation(int _pid1, int _pid2, double _similarity, string _rel_type)
{
	pid1 = _pid1;
	pid2 = _pid2;
	similarity = _similarity;
	rel_type = _rel_type;
}
AuthorRelation::~AuthorRelation()
{
}
AuthorRelation::AuthorRelation()
{
}