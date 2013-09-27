#pragma once
#include "aminer.hpp"
#include "indexed_graph.hpp"

struct AMinerData : public IndexedGraph {
    AMinerData(char const * prefix) : IndexedGraph(prefix) {
        buildIndexFor("Author", authorDocExtractor);
        buildIndexFor("Publication", pubDocExtractor);
        buildIndexFor("JConf", jconfDocExtractor);
    }

    static std::map<std::string, std::string> authorDocExtractor(sae::io::VertexIterator* it) {
        auto d = sae::serialization::convert_from_string<Author>(it->Data());
        std::string name;
        for (auto& s : d.names) {
            name += " " + s;
        }
        return std::map<std::string, std::string>{{"name", name}};
    }

    static std::map<std::string, std::string> pubDocExtractor(sae::io::VertexIterator* it) {
        auto d = sae::serialization::convert_from_string<Publication>(it->Data());
        return std::map<std::string, std::string>{{"abstract", d.title + d.abstract}};
    }

    static std::map<std::string, std::string> jconfDocExtractor(sae::io::VertexIterator* it) {
        auto d = sae::serialization::convert_from_string<JConf>(it->Data());
        return std::map<std::string, std::string>{{"name", d.name}};
    }
};
