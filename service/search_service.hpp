#include "zrpc/zrpc.hpp"
#include "indexing/search.hpp"

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
    SearchServiceBase(std::unique_ptr<IndexedGraph>&& ig) : ig(std::move(ig)) {
    }

protected:
    void fillSearchResponse(const aminer::EntitySearchRequest& request, aminer::EntitySearchResponse& response, const indexing::SearchResult& result) {
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

    virtual void fillEntity(aminer::DetailedEntity*, sae::io::VertexIterator*) = 0;

    std::unique_ptr<IndexedGraph> ig;
};
