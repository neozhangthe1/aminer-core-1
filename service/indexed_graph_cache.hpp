#include "indexed_graph.hpp"
#include "aminer_data.hpp"

struct IndexedGraphCache {
public:
    static IndexedGraph* getGraph(const char* name) {
        if (ig == nullptr) {
            std::unique_ptr<AMinerData> graph(new AMinerData(name));
            ig = std::move(graph);
        }
        return ig.get();
    }

private:
    IndexedGraphCache() {
        ig = nullptr;
    }
    unique_ptr<IndexedGraph> ig;
};