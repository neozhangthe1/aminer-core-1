#include "glog/logging.h"
#include "indexing/search.hpp"
#include "interface.pb.h"
#include "../search/event.hpp"
#include "../search/entity_searcher.hpp"
#include "search_service.hpp"
#include "../search/aminer_data.hpp"

using namespace std;
using namespace aminer;
using namespace indexing;
using namespace sae::io;
using namespace zrpc;

DEFINE_string(aminer, "aminer", "aminer data prefix");

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

struct AuthorService : public SearchServiceBase {
    AuthorService(std::unique_ptr<IndexedGraph>&& ig) : SearchServiceBase(std::move(ig)) {
    }

    SERVICE(searchAuthor, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Author", 1.0}}, WeightedType{{"Author", 1.0}, {"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }
};

struct ConfService : public SearchServiceBase {
    ConfService(std::unique_ptr<IndexedGraph>&& ig) : SearchServiceBase(std::move(ig)) {
    }

    SERVICE(searchConferences, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"JConf", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }
};

struct PubService : public SearchServiceBase {
    PubService(std::unique_ptr<IndexedGraph>&& ig) : SearchServiceBase(std::move(ig)) {
    }

    SERVICE(searchPublications, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Publication", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }
};

struct AMinerService : public SearchServiceBase {
    AMinerService(std::unique_ptr<IndexedGraph>&& ig) : SearchServiceBase(std::move(ig)) {
    }

    SERVICE(AuthorSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Author", 1.0}}, WeightedType{{"Author", 1.0}, {"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(JConfSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"JConf", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(PublicationSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Publication", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(InfluenceSearchByAuthor, EntitySearchRequest, InfluenceSearchResponse) {
        auto aid = stoi(request.query());
        response.set_entity_id(aid);

        auto vit = ig->g->Vertices();
        vit->MoveTo(aid);
        for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
            if (eit->TypeName() == "Influence") {
                auto ai = parse<AuthorInfluence>(eit->Data());
                Influence *inf = response.add_influence();
                inf->set_id(eit->TargetId());
                inf->set_topic(ai.topic);
                inf->set_score(ai.score);
            }
        }
        for (auto eit = vit->InEdges(); eit->Alive(); eit->Next()) {
            if (eit->TypeName() == "Influence") {
                auto ai = parse<AuthorInfluence>(eit->Data());
                Influence *inf = response.add_influenced_by();
                inf->set_id(eit->SourceId());
                inf->set_topic(ai.topic);
                inf->set_score(ai.score);
            }
        }
        return true;
    }

protected:
    void fillEntity(DetailedEntity* de, VertexIterator* vi) {
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
};

#define ADD_METHOD(name) server->addMethod(#name, b(&AMinerService::name))

static void init(void *sender, void *args) {
    RpcServer *server = reinterpret_cast<RpcServer *>(args);
    LOG(INFO) << "loading aminer graph: " << FLAGS_aminer;
    unique_ptr<AMinerData> ig(new AMinerData(FLAGS_aminer.c_str()));
    auto *service = new AMinerService(std::move(ig));
    auto b = zrpc::make_binder(*service);
    ADD_METHOD(AuthorSearch);
    ADD_METHOD(PublicationSearch);
    ADD_METHOD(JConfSearch);
    ADD_METHOD(InfluenceSearchByAuthor);
    LOG(INFO) << "aminer initialized. ";
}

REGISTER_EVENT(init_service, init);
