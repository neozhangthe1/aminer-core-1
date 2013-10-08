#pragma once
#include <string>
#include <vector>
#include "serialization/serialization.hpp"

using std::vector;
using std::string;

struct Author {
    int id;
    vector<string> names;
    string position;
    string affiliation;
    int h_index;
    int citation_number;
    int publication_number;
    vector<string> topics;
    string imgurl;
};

struct Publication {
    int id;
    string pubkey;
    int year;
    string title;
    string abstract;
    int jconf;
    int citation_number;
    vector<string> topics;
};

struct JConf {
    int id;
    string name;
};

struct AuthorPublication {
    // the position-th author in the author list
    int position;
};

struct PublicationJConf {
    // Noop
};

struct AuthorInfluence {
    int topic;
    double score;
};

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, Author> {
                static void run(sae::serialization::OSerializeStream& ostr, Author& a) {
                    ostr << a.id << a.names << a.position << a.affiliation << a.h_index << a.citation_number << a.publication_number << a.topics << a.imgurl;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, Author> {
                static void run(sae::serialization::ISerializeStream& istr, Author& a) {
                    istr >> a.id >> a.names >> a.position >> a.affiliation >> a.h_index >> a.citation_number >> a.publication_number >> a.topics >> a.imgurl;
                }
            };
        }
    }
}

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, Publication> {
                static void run(sae::serialization::OSerializeStream& ostr, Publication& p) {
                    ostr << p.id << p.pubkey << p.year << p.title << p.abstract << p.jconf << p.citation_number << p.topics;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, Publication> {
                static void run(sae::serialization::ISerializeStream& istr, Publication& p) {
                    istr >> p.id >> p.pubkey >> p.year >> p.title >> p.abstract >> p.jconf >> p.citation_number >> p.topics;
                }
            };
        }
    }
}

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, JConf> {
                static void run(sae::serialization::OSerializeStream& ostr, JConf& j) {
                    ostr << j.id << j.name;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, JConf> {
                static void run(sae::serialization::ISerializeStream& istr, JConf& j) {
                    istr >> j.id >> j.name;
                }
            };
        }
    }
}

