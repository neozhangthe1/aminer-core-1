#pragma once
#include "aminer.hpp"
#include "indexed_graph.hpp"
#include "gflags/gflags.h"

struct AMinerData : public IndexedGraph {
    AMinerData(char const * prefix) : IndexedGraph(prefix) {
        buildIndexFor("Author", authorDocExtractor);
        buildIndexFor("Publication", pubDocExtractor);
        buildIndexFor("JConf", jconfDocExtractor);
        auto vi = g->Vertices();
        LOG(INFO) << "building id map";
        while (vi->Alive()) {
            auto sae_id = vi->GlobalId();

            if (vi->TypeName() == "Author") {
                auto d = sae::serialization::convert_from_string<Author>(vi->Data());
                auto origin_id = d.id;
                idmap[std::make_pair("Author", origin_id)] = sae_id;
            } else if (vi->TypeName() == "Publication") {
                auto d = sae::serialization::convert_from_string<Publication>(vi->Data());
                auto origin_id = d.id;
                idmap[std::make_pair("Publication", origin_id)] = sae_id;
            } else if (vi->TypeName() == "JConf") {
                auto d = sae::serialization::convert_from_string<JConf>(vi->Data());
                auto origin_id = d.id;
                idmap[std::make_pair("JConf", origin_id)] = sae_id;
            }
            vi->Next();
        } 
        LOG(INFO) << "building id map finished";
    }

    static AMinerData& instance() {
        static AMinerData _amd(FLAGS_aminer.c_str());
        return _amd;
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
