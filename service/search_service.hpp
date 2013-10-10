#pragma once

#include "zrpc/zrpc.hpp"
#include "indexing/search.hpp"
#include "aminer.hpp"
#include "interface.pb.h"
#include "indexed_graph.hpp"

DECLARE_string(aminer);

namespace {
    string join(const string& sep, const vector<string>& values) {
        std::stringstream ss;
        for(size_t i = 0; i < values.size(); ++i)
        {
            if(i != 0)
                ss << sep;
            ss << values[i];
        }
        return ss.str();
    }
}

#define SERVICE(method, request_type, response_type) \
    bool method(const std::string& request_proto, std::string& response_proto) { \
        request_type request; \
        request.ParseFromString(request_proto); \
        response_type response; \
        bool ret = method##_impl(request, response); \
        response.SerializeToString(&response_proto); \
        return ret; \
    } \
    bool method##_impl(const request_type& request, response_type& response)

struct SearchServiceBase {
    SearchServiceBase(IndexedGraph* ig) : ig(ig) {
    }

protected:
    void fillSearchResponse(const mashaler::EntitySearchRequest& request, mashaler::EntitySearchResponse& response, const indexing::SearchResult& result) {
        int offset = request.has_offset() ? request.offset() : 0;
        int count = request.has_count() ? request.count() : 50;
        auto vi = ig->g->Vertices();
        for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
            vi->MoveTo(result[ri].docId);
            fillEntity(response.add_entity(), vi.get());
        }
        response.set_query(request.query());
        response.set_total_count(result.size());
    }

public:
    void fillEntity(mashaler::DetailedEntity* de, sae::io::VertexIterator* vi) {
        de->set_id(vi->GlobalId());
        if (vi->TypeName() == "Author") {
            auto author = parse<Author>(vi->Data());
            de->set_title(author.names[0]);
            de->set_original_id(author.id);
            string s = author.position;
            if (author.affiliation.size() > 0) {
                s += ", " + author.affiliation;
            }
            de->set_description(s);
            de->set_imgurl(author.imgurl);
            de->set_topics(join(",", author.topics));
            auto stat = de->add_stat();
            stat->set_type("h-index");
            stat->set_value(author.h_index);
            stat = de->add_stat();
            stat->set_type("citations");
            stat->set_value(author.citation_number);
            stat = de->add_stat();
            stat->set_type("publications");
            stat->set_value(author.publication_number);
        } else if (vi->TypeName() == "Publication") {
            auto pub = parse<Publication>(vi->Data());
            de->set_original_id(pub.id);
            de->set_title(pub.title);
            de->set_description(pub.abstract);
            de->set_topics(join(",", pub.topics));
            auto stat = de->add_stat();
            stat->set_type("year");
            stat->set_value(pub.year);
            stat = de->add_stat();
            stat->set_type("jconf");
            stat->set_value(pub.jconf);
            stat = de->add_stat();
            stat->set_type("citation");
            stat->set_value(pub.citation_number);

            auto re = de->add_related_entity();
            re->set_type("Author");
            for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
                if (ei->TypeName() == "Publish") {
                    re->add_id(ei->SourceId());
                }
            }
        } else if (vi->TypeName() == "JConf") {
            auto jc = parse<JConf>(vi->Data());
            de->set_title(jc.name);
            de->set_original_id(jc.id);
        }
    }

    IndexedGraph* ig;
};
