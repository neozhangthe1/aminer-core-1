#include "glog/logging.h"
#include "interface.pb.h"
#include "event.hpp"
#include "entity_searcher.hpp"
#include "search_service.hpp"
#include "aminer_data.hpp"
#include "indexed_graph_cache.hpp"

using namespace std;
using namespace mashaler;
using namespace indexing;
using namespace sae::io;
using namespace zrpc;

struct PubService : public SearchServiceBase {
    PubService(IndexedGraph* ig) : SearchServiceBase(ig) {
    }

    SERVICE(PubService_searchPublications, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig);
        auto results = searcher.search(WeightedType{{"Publication", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(PubService_getPublicationsByNaId, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig);
        auto results = searcher.get(WeightedType{{"Publication", 1.0}}, WeightedType{{"Author", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }
};

#define ADD_METHOD(name) server->addMethod(#name, b(&PubService::name))

static void init(void *sender, void *args) {
    RpcServer *server = reinterpret_cast<RpcServer *>(args);
    LOG(INFO) << "loading aminer graph for pubservice.";
    IndexedGraphCache& gc = IndexedGraphCache::instance();
    auto *service = new PubService(gc.getGraph());
    auto b = zrpc::make_binder(*service);
    ADD_METHOD(PubService_searchPublications);
    ADD_METHOD(PubService_getPublicationsByNaId);
    LOG(INFO) << "publication service initialized.";
}

REGISTER_EVENT(init_pubservice, init);
