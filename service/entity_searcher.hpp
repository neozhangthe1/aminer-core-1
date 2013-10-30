#pragma once
#include <map>

#include "indexed_graph.hpp"

using WeightedType = std::map<string, double>;

struct EntitySearcher {

    EntitySearcher(IndexedGraph* ig)
        : ig(ig) {
    }

    indexing::SearchResult get(WeightedType queryType, WeightedType searchType, const std::string& query, int limit = 5000) {

        LOG(INFO) << "query: " << query;
        // output
        indexing::SearchResult result;

        // get entity by id
        std::istringstream ss(query);
        std::string token;

        while(std::getline(ss, token, ',')) {
            auto aid = stoi(token);
            auto it = searchType.begin();
            LOG(INFO) << "getting enity: " << it->first << aid;
            auto pos = ig->idmap.find(std::make_pair(it->first,aid));
            auto sid = pos->second;
            LOG(INFO) << "sae id: " << sid;
            auto vi = ig->g->Vertices();
            vi->MoveTo(sid);
            it = queryType.begin();
            LOG(INFO) << "query type: " << it->first;
            if(vi->TypeName() == it->first){
                result.push_back(indexing::QueryItem{static_cast<int>(sid), 0});
            } else{
                for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
                    if (ei->Source()->TypeName() == it->first){
                        result.push_back(indexing::QueryItem{static_cast<int>(ei->SourceId()), 0});
                    }
                }
                for (auto ei = vi->OutEdges(); ei->Alive(); ei->Next()) {
                    if (ei->Target()->TypeName() == it->first){
                        result.push_back(indexing::QueryItem{static_cast<int>(ei->TargetId()), 0});
                    }
                }
            }
            
            LOG(INFO) << "getting enity: " << result.size();
        }

        // std::sort(result.begin(), result.end());
        if (result.size() > limit) {
            result.resize(limit);
        }

        LOG(INFO) << "size: " << result.size();

        return result;
    }

    indexing::SearchResult search(WeightedType queryType, WeightedType searchType, const std::string& query, int limit = 5000) {

        // initial scores
        std::map<sae::io::vid_t, double> scores;
        for (auto& kv : searchType) {
            auto result = ig->search(kv.first.c_str(), query, limit);
            for (auto& rs : result) {
                scores[rs.docId] += rs.score * kv.second;
            }
        }

        // expand & rerank
        expand(scores);

        // output
        indexing::SearchResult result;

        for (auto& kv : scores) {
            auto vi = ig->g->Vertices();
            vi->MoveTo(kv.first);
            auto it = queryType.find(vi->TypeName());
            if (it != queryType.end()) {
                result.push_back(indexing::QueryItem{static_cast<int>(kv.first), kv.second * it->second});
            }
        }

        std::sort(result.begin(), result.end());
        if (result.size() > limit) {
            result.resize(limit);
        }

        return result;
    }

    indexing::SearchResult getAuthorByName(const std::string& query) {
        return ig->search("Author", query, 5000);
    }

    indexing::SearchResult getJConfByName(const std::string& query) {
        return ig->search("JConf", query, 5000);
    }


protected:
    virtual void expand(std::map<sae::io::vid_t, double>& scores) {
        auto vi = ig->g->Vertices();
        std::map<sae::io::vid_t, double> new_scores;
        for (auto& kv : scores) {
            vi->MoveTo(kv.first);
            auto edges_count = vi->InEdgeCount() + vi->OutEdgeCount();
            // XXX temporary fix, keep the majority of the score to make the iteration stable.
            auto out = kv.second * 0.15 / edges_count;
            new_scores[kv.first] += kv.second * 0.85;
            for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
                new_scores[ei->SourceId()] += out;
            }
            for (auto ei = vi->OutEdges(); ei->Alive(); ei->Next()) {
                new_scores[ei->TargetId()] += out;
            }
        }
        swap(scores, new_scores);
    }

    const IndexedGraph* ig;
};
