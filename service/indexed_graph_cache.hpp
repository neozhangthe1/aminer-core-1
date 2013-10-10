#pragma once

#include "indexed_graph.hpp"
#include "aminer_data.hpp"

struct IndexedGraphCache {
public:
    static IndexedGraphCache& instance() {
        static IndexedGraphCache gc;
        return gc;
    }

    AMinerData* getGraph(const char* name) {
        if (ig == nullptr) {
            std::unique_ptr<AMinerData> graph(new AMinerData(name));
            ig = graph.get();
        }
        return ig;
    }

private:    
    IndexedGraphCache() {
        ig = nullptr;
    }

    AMinerData* ig;
};