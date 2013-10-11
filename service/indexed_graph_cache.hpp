#pragma once

#include "indexed_graph.hpp"
#include "aminer_data.hpp"

struct IndexedGraphCache {
public:
    static IndexedGraphCache& instance() {
        static IndexedGraphCache gc;
        return gc;
    }

    AMinerData* getGraph() {
        return ig;
    }

//private:    
    IndexedGraphCache() {
        AMinerData* graph = &AMinerData::instance();
        ig = graph;
    }

    AMinerData* ig;
};