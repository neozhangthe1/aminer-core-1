#pragma once

#include "JConfMap.h"
#include "interface.pb.h"
#include "document.h"
#include <utility>
#include <boost/array.hpp>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "AuthorAdapter.h"
#include "SearchFilter.h"
#include "AuthorRelation.h"

namespace ProtoConverter
{
	void convert(const google::protobuf::RepeatedField<google::protobuf::int32 >& from, vector<int>& to);

	bool convert(const AuthorRelation& relation, mashaler::AuthorRelation& newRel);

	bool convert(const JournalConferenceInfo& conf, mashaler::JournalConference& newConf);

	bool convert(DocumentCollection* collection, Document& doc, google::protobuf::Message& msg, const google::protobuf::RepeatedPtrField<string>& fields);

	bool convertPub(DocumentCollection* collection, Document& doc, google::protobuf::Message& newPub, const google::protobuf::RepeatedPtrField<string>& fields);

	bool convertPub(mashaler::Publication& m_publication, Publication& publication);

	bool convertAuthor(DocumentCollection* collection, Author& author, mashaler::Author& newAuthor, const google::protobuf::RepeatedPtrField<string>& fields);

	bool convert(Condition& con, const mashaler::Condition& con_interface);

	bool getUpdatedOrCreateNewAuthorWithMashaler(const mashaler::Author& mAuthor, DocumentCollection* collection, Author*& author_ptr, vector<const FieldInfo*>& affectedFields);

	inline bool hasField(const google::protobuf::Message& msg, const FieldInfo* fieldInfo)
	{
		switch(fieldInfo->type)
		{
		case FieldType::IntVector:
		case FieldType::StringVector:
			return msg.GetReflection()->FieldSize(msg,fieldInfo->protoField) > 0;
		default:
			return msg.GetReflection()->HasField(msg,fieldInfo->protoField);;
		}
	}

	template<typename T>
	void setField(T& t, const google::protobuf::Message& msg, const FieldInfo* fieldInfo)
	{
		auto field_ptr = t.getFieldPtr(fieldInfo);
		auto reflection = msg.GetReflection();
		auto protoField = fieldInfo->protoField;

		switch(fieldInfo->type)
		{
		case FieldType::Int:
			*((int*)field_ptr) = reflection->GetInt32(msg, protoField);
			break;
		case FieldType::Double:
			*((double*)field_ptr) = reflection->GetDouble(msg, protoField);
			break;
		case FieldType::String:
			*((string*)field_ptr) = reflection->GetString(msg, protoField);
			break;
		case FieldType::IntVector:
			((vector<int>*)field_ptr)->clear();
			for(int i = 0; i != reflection->FieldSize(msg, protoField); ++i)
				((vector<int>*)field_ptr)->push_back(reflection->GetRepeatedInt32(msg,protoField,i));
			break;
		case FieldType::StringVector:
			((vector<string>*)field_ptr)->clear();
			for(int i = 0; i != reflection->FieldSize(msg, protoField); ++i)
				((vector<string>*)field_ptr)->push_back(reflection->GetRepeatedString(msg,protoField,i));
			break;
		default:
			break;
		}
	}

	template<typename T>
	void getUpdatedField(DocumentCollection* collection, T& t, const google::protobuf::Message& msg, vector<const FieldInfo*>& updatedFields)
	{
		int field_size = collection->getFieldSize();
		updatedFields.clear();
		for(int i=0; i<field_size; ++i)
		{
			auto fieldInfo = collection->getFieldByIndex(i);
			if(hasField(msg, fieldInfo))
			{
				setField(t, msg, fieldInfo);
				updatedFields.push_back(fieldInfo);
			}
		}
	}
}