#pragma once

#include <atomic>
#include <map>
#include <string>

#include "glog/logging.h"
#include "thread_util.hpp"
#include "indexing/indexing.hpp"
#include "indexing/search.hpp"
#include "storage/mgraph.hpp"

namespace {
    template <typename T>
    inline T parse(const string& data) {
        return sae::serialization::convert_from_string<T>(data);
    }

    using ShardedIndex = std::vector<indexing::Index>;
    using DocExtractor = std::function<std::map<string, string>(sae::io::VertexIterator*)>;
    using TokenStreamer = std::function<TokenStream*(const std::string&)>;

    indexing::SearchResult shardedSearch(const std::vector<indexing::Index>& shards, const std::string& query, int limit, const TokenStreamer& tokenStream) {
        std::vector<indexing::SearchResult> results(shards.size());
        auto index_searcher = [&](int shard_id) {
            indexing::Searcher basic_searcher(shards[shard_id]);
            std::unique_ptr<TokenStream> stream(tokenStream(query));
            auto result = basic_searcher.search(stream.get());
            std::sort(result.begin(), result.end());
            results[shard_id] = std::move(result);
        };
        dispatch_thread_group(index_searcher, results.size());

        // merge the results.
        // TODO: improve this merge algorithm.
        for (int i = results.size() - 1; i > 0; i--) {
            int dst = i / 2;
            auto dsize = results[dst].size();
            results[dst].insert(results[dst].end(), results[i].begin(), results[i].end());
            std::inplace_merge(results[dst].begin(), results[dst].begin() + dsize, results[dst].end());
            results[i].clear();
            if (results[dst].size() > limit)
                results[dst].resize(limit);
        }

        return results[0];
    }

    ShardedIndex shardedIndex(sae::io::MappedGraph* g, char const * vertexType, DocExtractor extract, const TokenStreamer& tokenStream) {
        LOG(INFO) << "building sharded index...";
        int vertex_type_id = g->VertexTypeIdOf(vertexType);
        LOG(INFO) << "calculating average length...";

        // calc average length by sampling
        auto calc_avg_len = [&]() {
            double total_len = 0;
            int count = 0;
            int step = 100;
            for (auto ai = g->Vertices(); ai->Alive(); ai->MoveTo(ai->GlobalId() + step)) {
                if (ai->TypeId() == vertex_type_id) {
                    count++;
                    for (auto& item : extract(ai.get())) {
                        total_len += item.second.size();
                    }
                }
            }
            return total_len / count;
        };
        double avgLen = calc_avg_len();
        LOG(INFO) << "average length is: " << avgLen;

        LOG(INFO) << "building index...";
        ShardedIndex index_shards;
        const int shards = std::thread::hardware_concurrency();
        index_shards.resize(shards);

        auto offset = g->VerticesOfType(vertexType)->GlobalId();
        auto total = g->VertexCountOfType(vertexType);
        auto shard_size = total / shards;
        std::atomic<int> processed(0);
        auto index_builder = [&](int shard_id) {
            auto ai = g->Vertices();
            auto start = offset + shard_id * shard_size;
            auto end = offset + (shard_id + 1) * shard_size;
            LOG(INFO) << "shard " << shard_id << " processing range: " << start << " to " << end;
            for (auto i = start; i < end && ai->Alive(); ai->MoveTo(i)) {
                if (ai->TypeId() == vertex_type_id){
                    for (auto& kv : extract(ai.get())) {
                        std::unique_ptr<TokenStream> stream(tokenStream(kv.second));
                        index_shards[shard_id].addSingle(ai->GlobalId(), 0, stream.get(), avgLen);
                    }
                }
                // counter
                i++;
                if ((i - start) % 10000 == 0) {
                    processed += 10000;
                    LOG(INFO) << "[" << shard_id << "] progress: " << processed.load() << "/" << total << "\t" << (double)(processed.load()) / total;
                }
            }
            LOG(INFO) << "[" << shard_id << "] optimizing...";
            index_shards[shard_id].optimize();
            LOG(INFO) << "[" << shard_id << "] finished!";
        };
        dispatch_thread_group(index_builder, shards);
        LOG(INFO) << "index built!";
        return index_shards;
    }
}

struct IndexedGraph {

    IndexedGraph(char const * prefix, TokenStreamer tokenStream = ArnetAnalyzer::tokenStream)
        : g(sae::io::MappedGraph::Open(prefix)),
          tokenStream(tokenStream) {
    }

    virtual ~IndexedGraph() {
        LOG(INFO) << "Releasing graph data...";
    }

    void buildIndexFor(char const * vertexType, const DocExtractor& extract) {
        auto index = shardedIndex(g.get(), vertexType, extract, tokenStream);
        vertexIndices.emplace(std::string(vertexType), std::move(index));
    }

    indexing::SearchResult search(const char* vertexType, const string& query, int limit = 5000) const {
        auto it = vertexIndices.find(vertexType);
        if (it != vertexIndices.end()) {
            const ShardedIndex& index = it->second;
            return shardedSearch(index, query, limit, tokenStream);
        }
        return indexing::SearchResult();
    }

    template<typename T>
    T get(sae::io::vid_t id) const {
        auto vi = g->Vertices();
        vi->MoveTo(id);
        return parse<T>(vi->Data());
    }

    const std::unique_ptr<sae::io::MappedGraph> g;

protected:
    std::map<string, std::vector<indexing::Index>> vertexIndices;
    TokenStreamer tokenStream;
};

