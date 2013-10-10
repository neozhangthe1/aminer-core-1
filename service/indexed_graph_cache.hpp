#pragma once

#include "indexed_graph.hpp"
#include "aminer_data.hpp"

struct IndexedGraphCache {
public:
    static IndexedGraphCache& instance() {
        IndexedGraphCache::ig = nullptr;
        static IndexedGraphCache gc;
        return gc;
    }

    AMinerData* getGraph(const char* name) {
        return ig.get();
    }

//private:    
    IndexedGraphCache() {
        std::unique_ptr<AMinerData> graph(new AMinerData(FLAGS_aminer.c_str()));
        ig = std::move(graph);
    }

    static std::unique_ptr<AMinerData> ig;
};