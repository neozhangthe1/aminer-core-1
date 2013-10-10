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

struct JConfService : public SearchServiceBase {
    JConfService(IndexedGraph* ig) : SearchServiceBase(ig) {
    }

    SERVICE(ConfService_searchConferences, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig);
        auto results = searcher.search(WeightedType{{"JConf", 1.0}}, WeightedType{{"Publication", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

protected:
    void fillEntity(DetailedEntity* de, VertexIterator* vi) {
        de->set_id(vi->GlobalId());
        auto jc = parse<JConf>(vi->Data());
        de->set_title(jc.name);
        de->set_original_id(jc.id);
    }
};

#define ADD_METHOD(name) server->addMethod(#name, b(&JConfService::name))

static void init(void *sender, void *args) {
    RpcServer *server = reinterpret_cast<RpcServer *>(args);
    LOG(INFO) << "loading aminer graph for jconf service.";
    IndexedGraphCache& gc = IndexedGraphCache::instance();
    auto *service = new JConfService(gc.getGraph(FLAGS_aminer.c_str()));
    auto b = zrpc::make_binder(*service);
    ADD_METHOD(ConfService_searchConferences);
    LOG(INFO) << "jconf service initialized. ";
}

REGISTER_EVENT(init_confservice, init);
