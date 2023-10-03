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
    unsigned u, v;
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
        if (u > maxNode) maxNode = u;
        if (v > maxNode) maxNode = v;
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
    sort(std::execution::par, tmpEdgeList.begin(), tmpEdgeList.end(), compEdgeNodePSO);

    // TODO: use omp parallel for to handle each label
    // Can I do it without sort?
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
    sort(std::execution::par, tmpEdgeList.begin(), tmpEdgeList.end(), compEdgeNodePOS);

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

// Union the results in the list to get new result
void QueryResult::assignAsUnion(const std::vector<QueryResult> &qrList) {
    this->tryNew();
    MappedCSR *curCsrPtr = nullptr, *innerCsrPtr = nullptr;
    size_t numQr = qrList.size();
    for (size_t i = 0; i < numQr; i++) {
        if (qrList[i].hasEpsilon)
            this->hasEpsilon = true;
        curCsrPtr = qrList[i].csrPtr;
        for (const auto &pr : curCsrPtr->v2idx) {
            size_t v = pr.first, vIdx = pr.second;
            if (this->csrPtr->v2idx.find(v) == this->csrPtr->v2idx.end()) {
                this->csrPtr->v2idx[v] = this->csrPtr->offset.size();
                this->csrPtr->offset.emplace_back(this->csrPtr->adj.size());
                size_t adjStart = curCsrPtr->offset[vIdx], adjEnd = vIdx < curCsrPtr->n - 1 ? curCsrPtr->offset[vIdx + 1] : curCsrPtr->adj.size();
                move(curCsrPtr->adj.begin() + adjStart, curCsrPtr->adj.begin() + adjEnd, std::back_inserter(this->csrPtr->adj));
                for (size_t j = i + 1; j < numQr; j++) {
                    innerCsrPtr = qrList[j].csrPtr;
                    auto it = innerCsrPtr->v2idx.find(v);
                    if (it != innerCsrPtr->v2idx.end()) {
                        size_t vIdx2 = it->second;
                        size_t adjStart2 = innerCsrPtr->offset[vIdx2], adjEnd2 = vIdx2 < innerCsrPtr->n - 1 ? innerCsrPtr->offset[vIdx2 + 1] : innerCsrPtr->adj.size();
                        move(innerCsrPtr->adj.begin() + adjStart2, innerCsrPtr->adj.begin() + adjEnd2, std::back_inserter(this->csrPtr->adj));
                    }
                }
            }
        }
    }
    this->csrPtr->n = this->csrPtr->v2idx.size();
    this->csrPtr->m = this->csrPtr->adj.size();
}

// If encounter * or ? type:
// If left & right both has epsilon, mark as has epsilon;
// If only left (right) has epsilon, add all the right (left) results into the final result
void QueryResult::assignAsJoin(const QueryResult &qrLeft, const QueryResult &qrRight) {
    this->tryNew();
    for (const auto &pr : qrLeft.csrPtr->v2idx) {
        unordered_set<size_t> exist;
        size_t v = pr.first, vIdx = pr.second;
        size_t adjStart = qrLeft.csrPtr->offset[vIdx], adjEnd = vIdx < qrLeft.csrPtr->n - 1 ? qrLeft.csrPtr->offset[vIdx + 1] : qrLeft.csrPtr->adj.size();
        for (size_t i = adjStart; i < adjEnd; i++) {
            size_t nextNode = qrLeft.csrPtr->adj[i];
            auto it = qrRight.csrPtr->v2idx.find(nextNode);
            if (it != qrRight.csrPtr->v2idx.end()) {
                size_t nextNodeIdx = it->second;
                size_t adjStart2 = qrRight.csrPtr->offset[nextNodeIdx], adjEnd2 = nextNodeIdx < qrRight.csrPtr->n - 1 ? qrRight.csrPtr->offset[nextNodeIdx + 1] : qrRight.csrPtr->adj.size();
                for (size_t j = adjStart2; j < adjEnd2; j++) {
                    size_t nextNextNode = qrRight.csrPtr->adj[j];
                    if (exist.find(nextNextNode) == exist.end()) {
                        exist.emplace(nextNextNode);
                        this->csrPtr->adj.emplace_back(nextNextNode);
                    }
                }
            }
        }
        if (!qrLeft.hasEpsilon && qrRight.hasEpsilon) {
            for (size_t i = adjStart; i < adjEnd; i++) {
                size_t nextNode = qrLeft.csrPtr->adj[i];
                if (exist.find(nextNode) == exist.end()) {
                    exist.emplace(nextNode);
                    this->csrPtr->adj.emplace_back(nextNode);
                }
            }
        } else if (qrLeft.hasEpsilon && !qrRight.hasEpsilon) {
            auto it = qrRight.csrPtr->v2idx.find(v);
            if (it != qrRight.csrPtr->v2idx.end()) {
                size_t vIdx2 = it->second;
                size_t adjStart2 = qrRight.csrPtr->offset[vIdx2], adjEnd2 = vIdx2 < qrRight.csrPtr->n - 1 ? qrRight.csrPtr->offset[vIdx2 + 1] : qrRight.csrPtr->adj.size();
                for (size_t j = adjStart2; j < adjEnd2; j++) {
                    size_t nextNextNode = qrRight.csrPtr->adj[j];
                    if (exist.find(nextNextNode) == exist.end()) {
                        exist.emplace(nextNextNode);
                        this->csrPtr->adj.emplace_back(nextNextNode);
                    }
                }
            }
        }
        if (!exist.empty()) {
            this->csrPtr->v2idx[v] = this->csrPtr->offset.size();
            this->csrPtr->offset.emplace_back(this->csrPtr->adj.size() - exist.size());
        }
    }
    if (qrLeft.hasEpsilon && !qrRight.hasEpsilon) {
        // Handle the remaining results on the right
        for (const auto &pr : qrRight.csrPtr->v2idx) {
            size_t v = pr.first;
            if (this->csrPtr->v2idx.find(v) == this->csrPtr->v2idx.end()) {
                this->csrPtr->v2idx[v] = this->csrPtr->offset.size();
                this->csrPtr->offset.emplace_back(this->csrPtr->adj.size());
                size_t adjStart = qrRight.csrPtr->offset[pr.second], adjEnd = pr.second < qrRight.csrPtr->n - 1 ? qrRight.csrPtr->offset[pr.second + 1] : qrRight.csrPtr->adj.size();
                move(qrRight.csrPtr->adj.begin() + adjStart, qrRight.csrPtr->adj.begin() + adjEnd, std::back_inserter(this->csrPtr->adj));
            }
        }
    } else if (qrLeft.hasEpsilon && qrRight.hasEpsilon)
        this->hasEpsilon = true;
    this->csrPtr->n = this->csrPtr->v2idx.size();
    this->csrPtr->m = this->csrPtr->adj.size();
}