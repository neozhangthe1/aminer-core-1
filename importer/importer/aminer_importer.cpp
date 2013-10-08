/**
 * This program imports the AMiner Dataset into SAE.
 * Currently we need the following data schema for the demo.
 *
 *  Author:
 *      Id
 *      Names
 *      Position
 *      Affiliation
 *      H-Index
 *      CitationNumber
 *      PublicationNumber
 *      Topics
 *      ImgUrl
 *
 *  Publication:
 *      Id
 *      Title
 *      Year
 *      Topics
 *
 *  Conference:
 *      JConfId
 *      ConferenceName
 *
 *  Relations:
 *      Publish: Author-Publication
 *      Appear: Publication-Conference
 *
 */

#include <iostream>
#include <fstream>
#include <unordered_map>
#include "csv_reader.hpp"
#include "storage/mgraph.hpp"
#include "../../service/aminer.hpp"

using namespace std;

unordered_map<int, int> aid_map;
unordered_map<int, int> pid_map;
unordered_map<int, int> jconf_map;

int getId(unordered_map<int, int>& map, int id) {
    auto it = map.find(id);
    if (it == map.end()) {
        int next = map.size();
        map.insert(make_pair(id, next));
        return next;
    } else {
        return it->second;
    }
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

#define AUTHOR_BASE 0
#define PUBLICATION_BASE (1LL << 32)
#define JCONF_BASE (1LL << 33)

int main() {
    sae::io::vid_t gvid = 0, geid = 0;

    // GraphBuilder is not efficient for this kind of large data.
    cerr << "graph builder..." << endl;
    sae::io::GraphBuilder<uint64_t> builder;
    builder.AddVertexDataType("Author");
    builder.AddVertexDataType("Publication");
    builder.AddVertexDataType("JConf");
    builder.AddEdgeDataType("Publish");
    builder.AddEdgeDataType("Appear");
    builder.AddEdgeDataType("Influence");

    // build Authors
    cerr << "building authors..." << endl;
    {
        ifstream authors_csv("authors.csv");
        CSVReader authors_reader(authors_csv);
        Author a;
        string names;
        vector<string> row;
        authors_reader.readrow(row); // header
        while (authors_reader.readrow(row)) {
            a.id = stoi(row[0]);
            a.names = split(row[1], ',');
            a.position = row[2];
            a.affiliation = row[3];
            a.imgurl = row[4];
            a.topics = split(row[5], ',');
            a.h_index = stoi(row[6]);
            a.publication_number = stoi(row[7]);
            a.citation_number = stoi(row[8]);
            aid_map[a.id] = gvid++;

            builder.AddVertex(a.id, a, "Author");
        }
    }

    // build Conferences
    cerr << "building conferences..." << endl;
    {
        ifstream jconfs("jconf_name.txt");
        JConf j;
        while (jconfs >> j.id) {
            getline(jconfs >> std::ws, j.name);
            jconf_map[j.id] = gvid++;
            builder.AddVertex(JCONF_BASE + j.id, j, "JConf");
        }
    }

    // build Publications
    cerr << "building publications and appear relations..." << endl;
    {
        ifstream pubs("publication.txt");
        ifstream pub_ext("publication_ext.txt");
        ifstream pub_topics("terms_given_publication.txt");

        int pid, tmp_id, tmp_pubtopic_id;
        string tmp_abstract, tmp_topics;
        tmp_id = tmp_pubtopic_id = -1;

        while (pubs >> pid) {
            Publication p;
            p.id = pid;
            if (p.id % 10000 == 0) {
                cerr << "publication" << p.id << std::endl;
            }
            getline(pubs.ignore(), p.title, '\t');
            pubs >> p.jconf;
            pubs >> p.year;

            string authors;
            getline(pubs.ignore(), authors, '\t');  // unused

            string citation_number;
            getline(pubs, citation_number);
            if (citation_number.size() > 0) {
                //cerr << "!!!" << citation_number << endl;
                p.citation_number = stoi(citation_number);
            } else {
                p.citation_number = -1;
            }

            if (p.id > tmp_id) {
                while (pub_ext >> tmp_id) {
                    getline(pub_ext.ignore(), tmp_abstract);
                    if (tmp_id >= p.id) break;
                }
            }

            if (p.id > tmp_pubtopic_id) {
                while (pub_topics >> tmp_pubtopic_id) {
                    getline(pub_topics.ignore(), tmp_topics);
                    if (tmp_id >= p.id) break;
                }
            }

            if (p.id == tmp_id) {
                p.abstract = tmp_abstract;
            }

            if (p.id == tmp_pubtopic_id) {
                p.topics = split(tmp_topics, '\t');
            }

            pid_map[p.id] = gvid++;
            builder.AddVertex(PUBLICATION_BASE + p.id, p, "Publication");

            auto jit = jconf_map.find(p.jconf);
            if (jit == jconf_map.end()) {
                if (p.jconf > 0)
                    cerr << "jconf not found: " << p.jconf << endl;
            } else {
                builder.AddEdge(PUBLICATION_BASE + p.id, JCONF_BASE + p.jconf, PublicationJConf(), "Appear");
            }
            if (pubs.fail()) {
                cerr << "Read failed!" << p.id << endl;
                pubs.clear(ios::failbit);
            }

            //cerr << "pub id: " << p.id << " % " << gvid << " % " << p.title << " % " << authors << " % "  << p.jconf << " % " << p.year << " % "<< p.citation_number << endl;
        }
    }

    // build relations
    cerr << "building publish relations..." << endl;
    {
        ifstream a2p("a2p_a.txt");
        AuthorPublication ap;
        int aid, pid;
        int ncount = 0;
        while (a2p >> aid >> pid) {
            auto ait = aid_map.find(aid);
            auto pit = pid_map.find(pid);
            if (ait == aid_map.end()) {
                cerr << "Author not found: " << aid << endl;
            }
            if (pit == pid_map.end()) {
                cerr << "Publication not found: " << pid << ", not found count: " << (++ncount) << endl;
            }
            if (ait == aid_map.end() || pit == pid_map.end()) {
                continue;
            }
            builder.AddEdge(AUTHOR_BASE + aid, PUBLICATION_BASE + pid, ap, "Publish");
        }
    }

    cerr << "building influence graph..." << endl;
    {
        ifstream influence("influence_all.txt");
        AuthorInfluence ai;
        int source, target;
        while (influence >> source >> target >> ai.topic >> ai.score) {
            if (aid_map.find(source) == aid_map.end() || aid_map.find(target) == aid_map.end()) {
                cerr << "Author not found: " << source << ", " << target << endl;
                continue;
            }
            builder.AddEdge(AUTHOR_BASE + source, AUTHOR_BASE + target, ai, "Influence");
        }
    }

    cerr << "saving graph..." << endl;
    // save graph
    builder.Save("aminer");
    cerr << "saved graph" << endl;
    return 0;
}
