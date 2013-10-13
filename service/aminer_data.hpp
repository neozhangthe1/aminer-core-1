#pragma once
#include "aminer.hpp"
#include "indexed_graph.hpp"
#include "gflags/gflags.h"
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

DECLARE_string(index);
DECLARE_string(idmap);

namespace {

    const char * concat(const char * a, const char * b) {
        static char buf[1024];
        std::snprintf(buf, sizeof(buf), "%s%s", a, b);
        return buf;
    }

    const char * concat(const char * a, const int b) {
        static char buf[1024];
        std::snprintf(buf, sizeof(buf), "%s%d", a, b);
        return buf;
    }
}

struct AMinerData : public IndexedGraph {
    AMinerData(char const * prefix) : IndexedGraph(prefix) {
        if (serializeIndex()) {
            LOG(INFO) << "Load serialized index..";
            std::ifstream finv(FLAGS_index.c_str());
            sae::serialization::ISerializeStream decoderv(&finv);
            LOG(INFO) << "Deserializing...";
            decoderv >> vertexIndices;
            finv.close();
            LOG(INFO) << "Serialized index loaded..";
        }

        if (serializeIDMap()) {
            LOG(INFO) << "Load serialized idmap..";
            std::ifstream finv(FLAGS_idmap.c_str());
            sae::serialization::ISerializeStream decoderv(&finv);
            LOG(INFO) << "Deserializing...";
            decoderv >> idmap;
            finv.close();
            LOG(INFO) << "Serialized idmap loaded..";
        }
    }

    bool serializeIndex() {
        struct stat buf;
        int result;
        result =stat(FLAGS_index.c_str(), &buf );

        if (result != 0) {
            LOG(INFO) << "Serialized index file not found";
        }
        else {
            struct stat ori;
            stat(concat(FLAGS_aminer.c_str(), ".meta"), &ori);
            time_t i = buf.st_mtime;
            time_t o = ori.st_mtime;
            if (i > o)
                return true;
        }
        buildIndexFor("Author", authorDocExtractor);
        buildIndexFor("Publication", pubDocExtractor);
        buildIndexFor("JConf", jconfDocExtractor);
        /*
        LOG(INFO) << "Start serializing index..";
        std::ofstream fout("aminer.index", std::fstream::binary);
        sae::serialization::OSerializeStream encoder(&fout);
        encoder << vertexIndices;
        fout.close();
        LOG(INFO) << "Serializing index finished..";
        */
        return false;
    }

    bool serializeIDMap() {
        struct stat buf;
        int result;
        result =stat(FLAGS_idmap.c_str(), &buf );

        if (result != 0) {
            LOG(INFO) << "Serialized idmap file not found";
        }
        else {
            struct stat ori;
            stat(concat(FLAGS_aminer.c_str(), ".meta"), &ori);
            time_t i = buf.st_mtime;
            time_t o = ori.st_mtime;
            if (i > o)
                return true;
        }
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
        LOG(INFO) << "Start serializing idmap..";
        std::ofstream fout("aminer.idmap", std::fstream::binary);
        sae::serialization::OSerializeStream encoder(&fout);
        encoder << idmap;
        fout.close();
        LOG(INFO) << "Serializing idmap finished..";
        return false;
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
