#include "glog/logging.h"
#include "interface.pb.h"
#include "event.hpp"
#include "entity_searcher.hpp"
#include "search_service.hpp"
#include "aminer_data.hpp"
#include "indexed_graph_cache.hpp"

using namespace std;
using namespace aminer;
using namespace indexing;
using namespace sae::io;
using namespace zrpc;

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

struct PubService : public SearchServiceBase {
    PubService(IndexedGraph* ig) : SearchServiceBase(ig) {
    }

    SERVICE(PubService_searchPublications, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig);
        auto results = searcher.search(WeightedType{{"Publication", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

protected:
    void fillEntity(DetailedEntity* de, VertexIterator* vi) {
        de->set_id(vi->GlobalId());
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
    }
};

#define ADD_METHOD(name) server->addMethod(#name, b(&PubService::name))

static void init(void *sender, void *args) {
    RpcServer *server = reinterpret_cast<RpcServer *>(args);
    LOG(INFO) << "loading aminer graph for pubservice.";
    IndexedGraphCache& gc = IndexedGraphCache::instance();
    auto *service = new PubService(gc.getGraph(FLAGS_aminer.c_str()));
    auto b = zrpc::make_binder(*service);
    ADD_METHOD(PubService_searchPublications);
    LOG(INFO) << "publication service initialized.";
}

REGISTER_EVENT(init_pubservice, init);
