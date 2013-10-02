#include "InterfaceUtils.h"

#include "PublicationAdapter.h"
#include "AuthorAdapter.h"
#include "IndexResource.h"

#include <functional>
#include <boost/array.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace ProtoConverter
{
	void convert(const google::protobuf::RepeatedField<google::protobuf::int32>& from, vector<int>& to)
	{
		to.reserve(from.size());
		for(int i=0;i<from.size();++i)
			to.push_back(from.Get(i));
	}

	bool convert(const AuthorRelation& relation, mashaler::AuthorRelation& newRel)
	{
		newRel.set_pid1(relation.pid1);
		newRel.set_pid2(relation.pid2);
		newRel.set_similarity(relation.similarity);
		newRel.set_rel_type(relation.rel_type);
		return true;
	}

	bool convert(const JournalConferenceInfo& conf, mashaler::JournalConference& newConf)
	{
		newConf.set_id(conf.ID);
		newConf.set_name(conf.name);
		newConf.set_alias(conf.alias);
		newConf.set_score(conf.score);
		return true;
	}

	bool convert(DocumentCollection* collection, Document& doc, google::protobuf::Message& msg, const google::protobuf::RepeatedPtrField<string>& fields)
	{
		int count = fields.size();
		if(!count)
		{
			return true;
		}

		const::google::protobuf::Reflection* reflection = msg.GetReflection();
		const FieldInfo* field = NULL;
		for(int i = 0; i!=count; ++i)
		{
			field = collection->getFieldByName(fields.Get(i));
			if(field)
			{
				switch(field->type)
				{
				case FieldType::Int:
					{
						reflection->SetInt32(&msg, field->protoField, *((int*)(doc.getFieldPtr(field))));
					}
					break;
				case FieldType::Double:
					{
						reflection->SetDouble(&msg, field->protoField, *((double*)(doc.getFieldPtr(field))));
					}
					break;
				case FieldType::String:
					{
						reflection->SetString(&msg, field->protoField, *((string*)(doc.getFieldPtr(field))));
					}
					break;
				case FieldType::IntVector:
					{
						vector<int>& vec = *((vector<int>*)(doc.getFieldPtr(field)));
						for (int value : vec)
						{
							reflection->AddInt32(&msg, field->protoField, value);
						}
					}
					break;
				case FieldType::StringVector:
					{
						vector<string>& vec = *((vector<string>*)(doc.getFieldPtr(field)));
						for (string& value : vec)
						{
							reflection->AddString(&msg, field->protoField, value);
						}
					}
					break;
				}
			}
			else
			{
				printf("Error! field (%s) not found!\n", fields.Get(i).c_str());
			}
		}
		return true;
	}

	bool convertPub(DocumentCollection* collection, Document& doc, google::protobuf::Message& msg, const google::protobuf::RepeatedPtrField<string>& fields)
	{
		int count = fields.size();
		if(!count)
		{
			return true;
		}

		const::google::protobuf::Reflection* reflection = msg.GetReflection();
		const FieldInfo* field = NULL;
		for(int i = 0; i!=count; ++i)
		{
			field = collection->getFieldByName(fields.Get(i));
			if(field)
			{
				if(field->name == "topic")
					((Publication*)(&doc))->confirm_topic();
				switch(field->type)
				{
				case FieldType::Int:
					{
						reflection->SetInt32(&msg, field->protoField, *((int*)(doc.getFieldPtr(field))));
					}
					break;
				case FieldType::Double:
					{
						reflection->SetDouble(&msg, field->protoField, *((double*)(doc.getFieldPtr(field))));
					}
					break;
				case FieldType::String:
					{
						reflection->SetString(&msg, field->protoField, *((string*)(doc.getFieldPtr(field))));
					}
					break;
				case FieldType::IntVector:
					{
						vector<int>& vec = *((vector<int>*)(doc.getFieldPtr(field)));
						for (int value : vec)
						{
							reflection->AddInt32(&msg, field->protoField, value);
						}
					}
					break;
				case FieldType::StringVector:
					{
						vector<string>& vec = *((vector<string>*)(doc.getFieldPtr(field)));
						for (string& value : vec)
						{
							reflection->AddString(&msg, field->protoField, value);
						}
					}
					break;
				}
			}
			else
			{
				printf("Error! field (%s) not found!\n", fields.Get(i).c_str());
			}
		}
		return true;
	}

	bool convertAuthor(DocumentCollection* collection, Author& author, mashaler::Author& newAuthor, const google::protobuf::RepeatedPtrField<string>& fields)
	{
		int count = fields.size();
		if(!count)
		{
			return true;
		}

		const::google::protobuf::Reflection* reflection = newAuthor.GetReflection();
		const FieldInfo* field = NULL;
		for(int i = 0; i!=count; ++i)
		{
			field = collection->getFieldByName(fields.Get(i));
			if(field)
			{
				switch(field->type)
				{
				case FieldType::Int:
					{
						reflection->SetInt32(&newAuthor, field->protoField, *((int*)(author.getFieldPtr(field))));
					}
					break;
				case FieldType::Double:
					{
						reflection->SetDouble(&newAuthor, field->protoField, *((double*)(author.getFieldPtr(field))));
					}
					break;
				case FieldType::String:
					{
						reflection->SetString(&newAuthor, field->protoField, *((string*)(author.getFieldPtr(field))));
					}
					break;
				case FieldType::IntVector:
					{
						vector<int>& vec = *((vector<int>*)(author.getFieldPtr(field)));
						for (int value : vec)
						{
							reflection->AddInt32(&newAuthor, field->protoField, value);
						}
					}
					break;
				case FieldType::StringVector:
					{
						vector<string>& vec = *((vector<string>*)(author.getFieldPtr(field)));
						for (string& value : vec)
						{
							reflection->AddString(&newAuthor, field->protoField, value);
						}
					}
					break;
				default:
					{
						if(fields.Get(i)=="feature")
						{
							mashaler::AuthorFeature* fptr = newAuthor.mutable_feature();

							fptr->set_most_cited_paper(author.feature.bestpaperId);

							mashaler::IntArray* rank_ptr = NULL;
							mashaler::DoubleArray* score_ptr = NULL;
							for(int i=0;i<author.feature.expertiseTopic.size();++i)
							{
								fptr->mutable_topics()->add_values(author.feature.expertiseTopic[i]);
								rank_ptr = fptr->add_ranks();
								score_ptr = fptr->add_scores();
								for(int j=0;j<AuthorFeatures::TOTAL_FEATURES_WITH_TOPIC;++j)
								{
									rank_ptr->add_values(author.feature.ranks[i][j]);
									score_ptr->add_values(author.feature.scores[i][j]);
								}
							}
						}
					}
				}
			}
			else
			{
				printf("Error! field (%s) not found!\n", fields.Get(i).c_str());
			}
		}
		return true;
	}

	bool convert(Condition& con, const mashaler::Condition& con_interface)
	{
		con.field = con_interface.field();
		con.op = con_interface.op();
		con.value = con_interface.value();
		return true;
	}

	namespace{
		inline Author* copyExistingAuthorOrCreate( const mashaler::Author& mAuthor, DocumentCollection* collection )
		{
			return mAuthor.has_naid() ? 
				new Author(*((Author*)collection->getDocumentByIndex(mAuthor.naid())))
				: new Author(collection);
		}

		/*void setAuthorField(Author& const author, const mashaler::Author& mAuthor, const FieldInfo* fieldInfo)
		{
			auto field_ptr = author.getFieldPtr(fieldInfo);
			auto reflection = mAuthor.GetReflection();
			auto protoField = fieldInfo->protoField;

			switch(fieldInfo->type)
			{
			case FieldType::Int:
				*((int*)field_ptr) = reflection->GetInt32(mAuthor, protoField);
				break;
			case FieldType::Double:
				*((double*)field_ptr) = reflection->GetDouble(mAuthor, protoField);
				break;
			case FieldType::String:
				*((string*)field_ptr) = reflection->GetString(mAuthor, protoField);
				break;
			case FieldType::IntVector:
				for(int i = 0; i != reflection->FieldSize(mAuthor, protoField); ++i)
					((vector<int>*)field_ptr)->push_back(reflection->GetRepeatedInt32(mAuthor,protoField,i));
				break;
			case FieldType::StringVector:
				for(int i = 0; i != reflection->FieldSize(mAuthor, protoField); ++i)
					((vector<string>*)field_ptr)->push_back(reflection->GetRepeatedString(mAuthor,protoField,i));
				break;
			default:
				break;
			}
		}*/
	}

	bool getUpdatedOrCreateNewAuthorWithMashaler(const mashaler::Author& mAuthor, DocumentCollection* collection, Author*& author_ptr, vector<const FieldInfo*>& affectedFields)
	{
		author_ptr = copyExistingAuthorOrCreate(mAuthor, collection);
		affectedFields = vector<const FieldInfo*>();

		for(int i = 0; i!=collection->getFieldSize(); ++i)
		{
			auto fieldInfo = collection->getFieldByIndex(i);
			if(hasField(mAuthor , fieldInfo))
			{
				setField(*author_ptr, mAuthor, fieldInfo);
				affectedFields.push_back(fieldInfo);
			}
		}
		return true;
	}

	bool convertPub(mashaler::Publication& m_publication, Publication& publication)
	{
		if(m_publication.has_id())
			publication.id = m_publication.id();
		if(m_publication.has_title())
			publication.title = m_publication.title();
		if(m_publication.has_type_id())
			publication.typeID = m_publication.type_id();
		if(m_publication.has_jconf_name())
			publication.jconf_name = m_publication.jconf_name();
		if(m_publication.has_jconf_alias())
			publication.jconf_alias = m_publication.jconf_alias();
		if(m_publication.has_start_page())
			publication.startpage = m_publication.start_page();
		if(m_publication.has_end_page())
			publication.endpage = m_publication.end_page();
		if(m_publication.has_year())
			publication.year = m_publication.year();
		if(m_publication.has_publisher_id())
			publication.publisherID = m_publication.publisher_id();
		if(m_publication.has_authors())
			publication.authors = m_publication.authors();

		boost::range::copy(m_publication.author_ids(), 
			back_inserter(publication.authorIDs));

		if(m_publication.has_dblpeelink())
			publication.dblpeelink = m_publication.dblpeelink();
		if(m_publication.has_dblplink())
			publication.dblplink = m_publication.dblpeelink();
		if(m_publication.has_key())
			publication.key = m_publication.key();
		if(m_publication.has_n_citations())
			publication.nCitations = m_publication.n_citations();

		return true;
	}

	
}