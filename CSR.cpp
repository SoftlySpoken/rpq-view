#include "CSR.h"
using namespace std;

bool compEdgeNodePSO(const EdgeNode &e1, const EdgeNode &e2) {
    return (e1.label < e2.label) || (e1.label == e2.label && e1.s < e2.s) \
        || (e1.label == e2.label && e1.s == e2.s && e1.t < e2.t);
}

bool compEdgeNodePOS(const EdgeNode &e1, const EdgeNode &e2) {
    return (e1.label < e2.label) || (e1.label == e2.label && e1.t < e2.t) \
        || (e1.label == e2.label && e1.t == e2.t && e1.s < e2.s);
}

void MultiLabelCSR::loadGraph(const std::string &filePath, LineSeq lineSeq) {
    FILE *f = nullptr;
    f = fopen(filePath.c_str(), "r");
    if (!f) {
        printf("cannot open file\n");
        exit(30);
    }

    // First pass: get n, m, numLabel, type2label (map in-file labels to consecutive ints)
    // Get tmpEdgeList in the first pass (no second pass). Assume edges are sorted by SPO
    unsigned u, v, maxVert = 0;
    double type;
    unordered_map<double, size_t> &type2label = this->label2idx;
    std::vector<EdgeNode> tmpEdgeList;

    int readRet = -1;
    while (true) {
        if (lineSeq == sop)
            readRet = fscanf(f, "%u%u%lf", &u, &v, &type);
        else if (lineSeq == spo)
            readRet = fscanf(f, "%u%lf%u", &u, &type, &v);
        if (readRet == -1)
            break;
        if (u > maxVert) maxVert = u;
        if (v > maxVert) maxVert = v;
        if (type2label.find(type) == type2label.end()) {
            unsigned nextLabel = (unsigned)type2label.size();
            type2label[type] = nextLabel;
            tmpEdgeList.emplace_back(nextLabel, u, v);
        } else
            tmpEdgeList.emplace_back(type2label[type], u, v);
    }
    this->outCsr.clear();
    this->inCsr.clear();
    this->outCsr.resize(type2label.size());
    this->inCsr.resize(type2label.size());
    fclose(f);

    vector<unsigned> *adjVecPtr = nullptr, *offsetVecPtr = nullptr;
    unordered_map<unsigned, unsigned> *v2idxPtr = nullptr;
    int curNode = -1, curLabel = -1;
    
    // Out
    // Sort tmpEdgeList by PSO
    sort(tmpEdgeList.begin(), tmpEdgeList.end(), compEdgeNodePSO);

    for (const auto &te : tmpEdgeList) {
        if (curLabel != int(te.label)) {
            if (curLabel != -1) {
                this->outCsr[curLabel].m = adjVecPtr->size();
                this->outCsr[curLabel].n = offsetVecPtr->size();
            }
            curLabel = te.label;
            adjVecPtr = &this->outCsr[curLabel].adj;
            offsetVecPtr = &this->outCsr[curLabel].offset;
            v2idxPtr = &this->outCsr[curLabel].v2idx;
            curNode = -1;
        }
        if (curNode != int(te.s)) {
            v2idxPtr->emplace(te.s, offsetVecPtr->size());
            offsetVecPtr->emplace_back(adjVecPtr->size());
            curNode = te.s;
        }
        adjVecPtr->emplace_back(te.t);
    }
    this->outCsr[curLabel].m = adjVecPtr->size();
    this->outCsr[curLabel].n = offsetVecPtr->size();

    adjVecPtr = nullptr;
    offsetVecPtr = nullptr;
    v2idxPtr = nullptr;
    curNode = -1;
    curLabel = -1;

    // In
    // Sort tmpEdgeList by POS
    sort(tmpEdgeList.begin(), tmpEdgeList.end(), compEdgeNodePOS);

    for (const auto &te : tmpEdgeList) {
        if (curLabel != int(te.label)) {
            if (curLabel != -1) {
                this->inCsr[curLabel].m = adjVecPtr->size();
                this->inCsr[curLabel].n = offsetVecPtr->size();
            }
            curLabel = te.label;
            adjVecPtr = &this->inCsr[curLabel].adj;
            offsetVecPtr = &this->inCsr[curLabel].offset;
            v2idxPtr = &this->inCsr[curLabel].v2idx;
            curNode = -1;
        }
        if (curNode != int(te.t)) {
            v2idxPtr->emplace(te.t, offsetVecPtr->size());
            offsetVecPtr->emplace_back(adjVecPtr->size());
            curNode = te.t;
        }
        adjVecPtr->emplace_back(te.s);
    }
    this->inCsr[curLabel].m = adjVecPtr->size();
    this->inCsr[curLabel].n = offsetVecPtr->size();
}

// v is the vertex ID in the original graph before mapping
void MappedCSR::getAdjIntervalByVert(unsigned v, AdjInterval &aitv) const {
    auto iter = v2idx.find(v);
    if (iter == v2idx.end()) {
        aitv.start = nullptr;
        aitv.len = 0;
        aitv.offset = 0;
        return;
    }
    aitv.start = &adj;
    unsigned idx = iter->second;
    size_t len = 0, curOff = offset[idx];
    if (idx == n - 1)
        len = m - curOff;
    else
        len = offset[idx + 1] - curOff;
    aitv.len = len;
    aitv.offset = curOff;
}

bool MappedCSR::operator == (const MappedCSR &c) const {
    if (n != c.n || m != c.m)
        return false;
    unordered_set<unsigned> curNei;
    unordered_map<unsigned, unsigned>::const_iterator it;
    AdjInterval aitv1, aitv2;
    for (const auto &pr : v2idx) {
        it = c.v2idx.find(pr.first);
        if (it == c.v2idx.end())
            return false;
        getAdjIntervalByVert(pr.first, aitv1);
        c.getAdjIntervalByVert(pr.first, aitv2);
        if (aitv1.len != aitv2.len)
            return false;
        curNei.clear();
        curNei.insert(aitv1.start->begin() + aitv1.offset, aitv1.start->begin() + aitv1.offset + aitv1.len);
        for (size_t i = 0; i < aitv2.len; i++) {
            if (curNei.find((*aitv2.start)[aitv2.offset + i]) == curNei.end())
                return false;
        }
    }
    return true;
}

void MultiLabelCSR::fillStats() {
    size_t labelCnt = outCsr.size();
    stats.outCnt.resize(labelCnt);
    stats.inCnt.resize(labelCnt);
    stats.outCooccur.resize(labelCnt);
    stats.inCooccur.resize(labelCnt);
    for (size_t i = 0; i < labelCnt; i++) {
        stats.outCnt[i].assign(labelCnt, 0);
        stats.inCnt[i].assign(labelCnt, 0);
        stats.outCooccur[i].assign(labelCnt, 0);
        stats.inCooccur[i].assign(labelCnt, 0);
    }

    for (size_t y = 0; y < labelCnt; y++) {
        for (const auto &pr : outCsr[y].v2idx) {
            size_t v = pr.first, idx = pr.second;
            size_t outDeg = idx < outCsr[y].n - 1 ? 
                outCsr[y].offset[idx + 1] - outCsr[y].offset[idx] :
                outCsr[y].m - outCsr[y].offset[idx];
            for (size_t x = 0; x < labelCnt; x++) {
                if (inCsr[x].v2idx.find(v) != inCsr[x].v2idx.end()) {
                    size_t inner = inCsr[x].v2idx.at(v);
                    size_t inDeg = inner < inCsr[x].n - 1 ?
                        inCsr[x].offset[inner + 1] - inCsr[x].offset[inner] :
                        inCsr[x].m - inCsr[x].offset[inner];
                    stats.outCnt[x][y] += inDeg;
                    stats.inCnt[y][x] += outDeg;
                }
                if (y != x && outCsr[x].v2idx.find(v) != outCsr[x].v2idx.end()) {
                    size_t inner = outCsr[x].v2idx.at(v);
                    size_t inner_outDeg = inner < outCsr[x].n - 1 ?
                        outCsr[x].offset[inner + 1] - outCsr[x].offset[inner] :
                        outCsr[x].m - outCsr[x].offset[inner];
                    stats.outCooccur[x][y] += inner_outDeg;
                }
            }
        }
        for (const auto &pr : inCsr[y].v2idx) {
            size_t v = pr.first;
            for (size_t x = 0; x < labelCnt; x++) {
                if (y != x && inCsr[x].v2idx.find(v) != inCsr[x].v2idx.end()) {
                    size_t inner = inCsr[x].v2idx.at(v);
                    size_t inDeg = inner < inCsr[x].n - 1 ?
                        inCsr[x].offset[inner + 1] - inCsr[x].offset[inner] :
                        inCsr[x].m - inCsr[x].offset[inner];
                    stats.inCooccur[x][y] += inDeg;
                }
            }
        }
    }
}