#pragma once
#include "Util.h"

enum LineSeq {sop, spo};

struct EdgeNode {
    unsigned label;
    int s;
    int t;
    EdgeNode() {}
    EdgeNode(unsigned label_, int s_, int t_): label(label_), s(s_), t(t_) {}
};

struct AdjInterval {
    const std::vector<unsigned> *start;
    size_t len;
    unsigned offset;
    AdjInterval(): start(nullptr), len(0), offset(0) { }
    AdjInterval(std::vector<unsigned> *start_, size_t len_): start(start_), len(len_) { }
    inline void print() {
        std::cout << len << std::endl;
        for (size_t i = 0; i < len; i++)
            std::cout << (*start)[i] << ' ';
        std::cout << std::endl;
    }
};

struct MappedCSR {
    unsigned n;
    unsigned m;
    std::vector<unsigned> adj;
    std::vector<unsigned> offset;
    std::unordered_map<unsigned, unsigned> v2idx;
    MappedCSR(): n(0), m(0) {}
    void getAdjIntervalByVert(unsigned v, AdjInterval &aitv) const;
    bool empty() const { return v2idx.empty(); }
    void print() const {
        for (const auto &pr : v2idx)
            std::cout << pr.first << ":" << pr.second << " ";
        std::cout << std::endl;
        for (size_t i = 0; i < n; i++)
            std::cout << offset[i] << " ";
        std::cout << std::endl;
        for (size_t i = 0; i < m; i++)
            std::cout << adj[i] << " ";
        std::cout << std::endl;
    }
    bool operator == (const MappedCSR &c) const;
    bool operator != (const MappedCSR &c) const { return !(*this == c); }
};

struct Statistics {
    std::vector<std::vector<size_t>> outCnt, inCnt; // outCnt[x][y]: #x-edges whose next hop has y-edges
    // outCooccur[x][y]: #x-edges whose start nodes also have y-edges. Note: when x==y, the cooccur probability is always 1
    std::vector<std::vector<size_t>> outCooccur, inCooccur;
};

struct MultiLabelCSR {
    std::vector<MappedCSR> outCsr, inCsr;
    std::unordered_map<double, size_t> label2idx;
    Statistics stats;
    unsigned maxNode;   // The maximum node id
    MultiLabelCSR(): maxNode(0) {}
    void loadGraph(const std::string &filePath, LineSeq lineSeq=sop);
    void fillStats();
};

struct QueryResult {
    MappedCSR *csrPtr;
    bool newed;
    bool hasEpsilon;    // include all (v, v) pairs, not explictly represented
    QueryResult(MappedCSR *csrPtr_, bool newed_): csrPtr(csrPtr_), newed(newed_), hasEpsilon(false) {}
};