#include "AndOrDag.h"
using namespace std;

template<typename T1, typename T2>
bool PairSecondLess(const pair<T1, T2> &p1, const pair<T1, T2> &p2) {
    return p1.second < p2.second;
}

template<typename T1, typename T2>
bool PairSecondGreater(const pair<T1, T2> &p1, const pair<T1, T2> &p2) {
    return p1.second > p2.second;
}

void AndOrDag::addWorkloadQuery(const std::string &q, size_t freq) {
    int ret = addQuery(q);
    if (ret >= 0) {
        useCnt[ret] += freq;
        workloadFreq[ret] += freq;
    }
}

int AndOrDag::addQuery(const std::string &q) {
    if (q.empty())
        return -1;
    const auto &it = q2idx.find(q);
    if (q2idx.find(q) != q2idx.end())
        return it->second;
    
    istringstream ifs(q);
    RpqErrorListener lstnr;
    antlr4::ANTLRInputStream input(ifs);
    rpqLexer lexer(&input);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lstnr);
    antlr4::CommonTokenStream tokens(&lexer);
    rpqParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&lstnr);
    rpqParser::PathContext *path = parser.path();
    
    // To implement cost estimation for concat and kleene, need to record start/end labels
    // Alternation: union of all child nodes' start/end labels (therefore needs to be vector)
    // Concat: start label of first child, end label of last child
    // Kleene: start label of child, end label of child
    // ?: start label of child, end label of child
    // Base case: start label = end label = current label
    addNode(true, 0);
    size_t ret = nodes.size() - 1, tmpIdx = 0;
    const auto &pathSequenceVec = path->pathSequence();
    if (pathSequenceVec.size() > 1) {
        addNode(false, 0);
        tmpIdx = nodes.size() - 1;
        addParentChild(ret, tmpIdx);
        for (const auto &pathSequence : pathSequenceVec) {
            size_t curRet = addQuery(pathSequence->getText());
            addParentChild(tmpIdx, curRet);
            nodes[ret].addStartLabel(nodes[curRet].getStartLabel());
            nodes[ret].addEndLabel(nodes[curRet].getEndLabel());
        }
        q2idx[q] = ret;
        return ret;
    }
    const auto &pathEltVec = pathSequenceVec[0]->pathElt();
    if (pathEltVec.size() > 1) {
        size_t sz = pathEltVec.size();
        string lStr = pathEltVec[0]->getText(), rStr = pathEltVec[1]->getText();
        for (size_t i = 2; i < sz; i++) {
            rStr += "/";
            rStr += pathEltVec[i]->getText();
        }
        for (size_t i = 0; i < sz - 1; i++) {
            addNode(false, 1);
            tmpIdx = nodes.size() - 1;
            addParentChild(ret, tmpIdx);
            size_t lRet = addQuery(lStr);
            size_t rRet = addQuery(rStr);
            addParentChild(tmpIdx, lRet);
            addParentChild(tmpIdx, rRet);
            if (i == 0)
                nodes[ret].addStartLabel(nodes[lRet].getStartLabel());
            if (i != sz - 2) {
                lStr += "/" + pathEltVec[i + 1]->getText();
                rStr = rStr.substr(rStr.find('/') + 1);
            } else
                nodes[ret].addEndLabel(nodes[rRet].getEndLabel());
        }
        q2idx[q] = ret;
        return ret;
    }
    const auto &pathMod = pathEltVec[0]->pathMod();
    if (!pathMod) {
        if (pathEltVec[0]->pathPrimary()->path()) {
            nodes.pop_back();
            ret = addQuery(pathEltVec[0]->pathPrimary()->path()->getText());
            return ret;
        } else {
            q2idx[q] = ret;
            bool isInv = q[q.size() - 2] == '-';
            double curLbl = stod(q.substr(1, q.size() - 2 - (isInv ? 1 : 0)));
            nodes[ret].addStartLabel(curLbl, isInv);
            nodes[ret].addEndLabel(curLbl, isInv);
            return ret;
        }
    } else {
        string pathModStr = pathMod->getText();
        tmpIdx = nodes.size();
        if (pathModStr == "?")
            addNode(false, 4);
        else if (pathModStr == "*")
            addNode(false, 2);
        else if (pathModStr == "+")
            addNode(false, 3);
        addParentChild(ret, tmpIdx);
        if (pathEltVec[0]->pathPrimary()->path()) {
            size_t curRet = addQuery(pathEltVec[0]->pathPrimary()->path()->getText());
            addParentChild(tmpIdx, curRet);
            nodes[ret].addStartLabel(nodes[curRet].getStartLabel());
            nodes[ret].addEndLabel(nodes[curRet].getEndLabel());
        }
        else {
            addNode(true, 0);
            addParentChild(tmpIdx, nodes.size() - 1);
            q2idx[pathEltVec[0]->pathPrimary()->getText()] = nodes.size() - 1;
            bool isInv = q[q.size() - 3] == '-';
            double curLbl = stod(q.substr(1, q.size() - 3 - (isInv ? 1 : 0)));
            nodes.back().addStartLabel(curLbl, isInv);
            nodes.back().addEndLabel(curLbl, isInv);
            nodes[ret].addStartLabel(curLbl, isInv);
            nodes[ret].addEndLabel(curLbl, isInv);
        }
        q2idx[q] = ret;
        return ret;
    }
}

void AndOrDag::initAuxiliary() {
    size_t numNodes = nodes.size();
    materialized.assign(numNodes, false);
    cost.assign(numNodes, 0);
    srcCnt.assign(numNodes, 0);
    dstCnt.assign(numNodes, 0);
    pairProb.assign(numNodes, 0);
    topoSort();
}

void AndOrDag::annotateLeafCostCard() {
    if (!csrPtr) {
        cerr << "Please set CSR pointer before calling annotateLeafCostCard()" << endl;
        return;
    }
    // Find leaf node by label and its inverse
    for (const auto &pr : csrPtr->label2idx) {
        double lbl = pr.first;
        size_t i = pr.second;
        string iriStr = "<" + to_string(size_t(lbl)) + ">", invIriStr = "<" + to_string(size_t(lbl)) + "->";
        auto it = q2idx.find(iriStr);
        size_t idx = 0;
        if (it != q2idx.end()) {
            idx = it->second;
            srcCnt[idx] = csrPtr->outCsr[i].n;
            dstCnt[idx] = csrPtr->inCsr[i].n;
            cost[idx] = csrPtr->outCsr[i].m;
            pairProb[idx] = float(cost[idx]) / float(srcCnt[idx] * dstCnt[idx]);
        }
        it = q2idx.find(invIriStr);
        if (it != q2idx.end()) {
            idx = it->second;
            srcCnt[idx] = csrPtr->inCsr[i].n;
            dstCnt[idx] = csrPtr->outCsr[i].n;
            cost[idx] = csrPtr->outCsr[i].m;
            pairProb[idx] = float(cost[idx]) / float(srcCnt[idx] * dstCnt[idx]);
        }
    }
}

void AndOrDag::plan() {
    if (!csrPtr) {
        cerr << "Please set CSR pointer before calling plan()" << endl;
        return;
    }
    size_t numNodes = nodes.size();
    for (size_t i = 0; i < numNodes; i++) {
        if (useCnt[i] > 0)
            planNode(i);
    }
    // Propagate useCnt downwards only once from all root nodes (those without parents)
    for (size_t i = 0; i < numNodes; i++) {
        if (nodes[i].getParentIdx().empty())
            propagateUseCnt(i);
    }
}

void AndOrDag::propagateUseCnt(size_t idx) {
    const auto &curChildIdx = nodes[idx].getChildIdx();
    size_t numChild = curChildIdx.size();
    if (numChild == 0)
        return;
    for (size_t i = 0; i < numChild; i++) {
        useCnt[curChildIdx[i]] += useCnt[idx];
        propagateUseCnt(curChildIdx[i]);
    }
}

void AndOrDag::planNode(size_t nodeIdx) {
    if (cost[nodeIdx] != 0)
        return;
    auto &curNode = nodes[nodeIdx];
    const auto &curChildIdx = curNode.getChildIdx();
    size_t numChild = curChildIdx.size();
    if (numChild == 0) {
        materialized[nodeIdx] = true;
        return;
    }
    for (size_t i = 0; i < numChild; i++)
        planNode(curChildIdx[i]);
    bool isEq = curNode.getIsEq();
    if (isEq) {
        size_t targetChild = curChildIdx[0];
        float minCost = cost[curChildIdx[0]];
        for (size_t i = 1; i < numChild; i++) {
            if (cost[curChildIdx[i]] < minCost) {
                minCost = cost[curChildIdx[i]];
                targetChild = curChildIdx[i];
            }
        }
        curNode.setTargetChild(targetChild);
        cost[nodeIdx] = minCost;
        srcCnt[nodeIdx] = srcCnt[targetChild];
        dstCnt[nodeIdx] = dstCnt[targetChild];
        pairProb[nodeIdx] = pairProb[targetChild];
    } else {
        char curOpType = curNode.getOpType();
        if (curOpType == 0) {
            // Alternation
            cost[nodeIdx] = 0;
            srcCnt[nodeIdx] = 0;
            dstCnt[nodeIdx] = 0;
            size_t curCard = 0;
            for (size_t i = 0; i < numChild; i++) {
                cost[nodeIdx] += cost[curChildIdx[i]];
                srcCnt[nodeIdx] += srcCnt[curChildIdx[i]];
                dstCnt[nodeIdx] += dstCnt[curChildIdx[i]];
                curCard += srcCnt[curChildIdx[i]] * dstCnt[curChildIdx[i]] * pairProb[curChildIdx[i]];
            }
            pairProb[nodeIdx] = float(curCard) / float(srcCnt[nodeIdx] * dstCnt[nodeIdx]);
        } else if (curOpType == 1) {
            // Concatenation: preserve the choice of left to right or right to left
            assert(numChild == 2);
            size_t lChild = curChildIdx[0], rChild = curChildIdx[1];
            float plan1 = 0, plan2 = 0; // plan1: cost2 + w1 * cost1; plan2: cost1 + w2 * cost2
            float cost1 = cost[lChild], cost2 = cost[rChild];
            const auto &lEnd = nodes[lChild].getEndLabel(), &rStart = nodes[rChild].getStartLabel();
            pair<float, float> lrW = getLeftRightWeight(lEnd, rStart);
            float w1 = lrW.first, w2 = lrW.second;
            plan1 = cost2 + w1 * cost1;
            plan2 = cost1 + w2 * cost2;
            if (plan1 < plan2) {
                curNode.setLeft2Right(false);
                cost[nodeIdx] = plan1;
            }
            else
                cost[nodeIdx] = plan2;
            srcCnt[nodeIdx] = w1 * srcCnt[lChild];
            dstCnt[nodeIdx] = w2 * dstCnt[rChild];
            float lProb = pairProb[lChild] * w1, rProb = pairProb[rChild] * w2;
            pairProb[nodeIdx] = lProb < rProb ? lProb : rProb;
        } else if (curOpType == 2 || curOpType == 3) {
            assert(numChild == 1);
            size_t curChild = curChildIdx[0];
            pair<float, float> lrW = getLeftRightWeight(nodes[curChild].getEndLabel(), nodes[curChild].getStartLabel());
            float w1 = lrW.first, w2 = lrW.second;
            float curW = w2;
            cost[nodeIdx] = cost[curChild];
            srcCnt[nodeIdx] = srcCnt[curChild];
            dstCnt[nodeIdx] = dstCnt[curChild];
            size_t curCard = srcCnt[curChild] * dstCnt[curChild] * pairProb[curChild];
            float curPairProb = pairProb[curChild];
            size_t curSrcCnt = srcCnt[curChild], curDstCnt = dstCnt[curChild];
            do {
                cost[nodeIdx] += curW * cost[curChild];
                curSrcCnt *= w1;
                curDstCnt *= w2;
                curPairProb = w1 < w2 ? w1 * curPairProb : w2 * curPairProb;
                curCard += curSrcCnt * curDstCnt * curPairProb;
                srcCnt[nodeIdx] += curSrcCnt;
                dstCnt[nodeIdx] += curDstCnt;
                curW *= w2;
            } while (curW >= 0.01 && curSrcCnt + curDstCnt > 0);
            pairProb[nodeIdx] = float(curCard) / float(srcCnt[nodeIdx] * dstCnt[nodeIdx]);
        } else if (curOpType == 4) {
            // ?
            assert(numChild == 1);
            cost[nodeIdx] = cost[curChildIdx[0]];
            srcCnt[nodeIdx] = srcCnt[curChildIdx[0]];
            dstCnt[nodeIdx] = dstCnt[curChildIdx[0]];
            pairProb[nodeIdx] = pairProb[curChildIdx[0]];
        }
    }
}

std::pair<float, float> AndOrDag::getLeftRightWeight(const std::vector<LabelOrInverse> &lEnd, const std::vector<LabelOrInverse> &rStart) {
    float w1 = 0, w2 = 0;
    for (const auto &le : lEnd) {
        auto it = csrPtr->label2idx.find(le.lbl);
        assert(it != csrPtr->label2idx.end());
        size_t lIdx = it->second;
        size_t lTotal = csrPtr->outCsr[lIdx].m;
        float w1_local = 0;
        for (const auto &rs : rStart) {
            it = csrPtr->label2idx.find(rs.lbl);
            assert(it != csrPtr->label2idx.end());
            size_t rIdx = it->second;
            size_t rTotal = csrPtr->outCsr[rIdx].m;
            if (!le.inv && !rs.inv) {
                w1_local += csrPtr->stats.outCnt[lIdx][rIdx];
                w2 += float(csrPtr->stats.inCnt[rIdx][lIdx]) / float(rTotal);
            } else if (!le.inv && rs.inv) {
                w1_local += csrPtr->stats.inCnt[lIdx][rIdx];
                w2 += float(csrPtr->stats.inCooccur[rIdx][lIdx]) / float(rTotal);
            } else if (le.inv && !rs.inv) {
                w1_local += csrPtr->stats.outCooccur[lIdx][rIdx];
                w2 += float(csrPtr->stats.outCooccur[rIdx][lIdx]) / float(rTotal);
            } else {
                w1_local += csrPtr->stats.inCnt[lIdx][rIdx];
                w2 += float(csrPtr->stats.outCnt[rIdx][lIdx]) / float(rTotal);
            }
        }
        w1 += w1_local / float(lTotal);
    }
    return make_pair(w1, w2);
}

void AndOrDag::topoSort() {
    // Maintain #parents of each node, take those with 0 as sorted, subtract 1 from its children's number
    size_t numNodes = nodes.size();
    vector<size_t> parentCnt(numNodes, 0);
    size_t numDone = 0;
    int curOrder = 0;
    queue<size_t> q;
    for (size_t i = 0; i < numNodes; i++) {
        parentCnt[i] = nodes[i].getParentIdx().size();
        if (parentCnt[i] == 0)
            q.push(i);
    }
    size_t curIdx = 0;
    while (!q.empty()) {
        curIdx = q.front();
        q.pop();
        nodes[curIdx].setTopoOrder(curOrder);
        curOrder++;
        numDone++;
        const auto &curChildIdx = nodes[curIdx].getChildIdx();
        for (size_t childIdx : curChildIdx) {
            parentCnt[childIdx]--;
            if (parentCnt[childIdx] == 0)
                q.push(childIdx);
        }
    }
    assert(numDone == numNodes);
}

void AndOrDag::replanWithMaterialize(const std::vector<size_t> &matIdx,
std::unordered_map<size_t, float> &node2cost, float &reducedCost) {
    reducedCost = 0;
    // matIdx & topoOrder
    priority_queue<pair<size_t, size_t>, vector<pair<size_t, size_t>>, decltype(&PairSecondLess<size_t, size_t>)> pq(PairSecondLess);
    for (size_t idx : matIdx)
        pq.emplace(idx, nodes[idx].getTopoOrder());
    while (!pq.empty()) {
        size_t curIdx = pq.top().first;
        pq.pop();
        updateNodeCost(curIdx, node2cost, reducedCost);
    }
}

void AndOrDag::applyChanges(const std::unordered_map<size_t, float> &node2cost) {
    for (const auto &pr : node2cost) {
        size_t nodeIdx = pr.first;
        cost[nodeIdx] = pr.second;
        // For eq nodes with multiple / op children, update targetChild if necessary
        if (!nodes[nodeIdx].getIsEq() && nodes[nodeIdx].getOpType() == 1) {
            const auto &parentIdx = nodes[nodeIdx].getParentIdx();
            std::unordered_map<size_t, float>::const_iterator it;
            for (size_t parent : parentIdx) {
                it = node2cost.find(parent);
                if (it != node2cost.end() && it->second == pr.second)
                    nodes[parent].setTargetChild(nodeIdx);
            }
        }
    }
}

void AndOrDag::updateNodeCost(size_t nodeIdx, std::unordered_map<size_t, float> &node2cost,
float &reducedCost, float updateCost) {
    float realUpdateCost = updateCost < 0 ? float(srcCnt[nodeIdx] * dstCnt[nodeIdx]) * pairProb[nodeIdx] : updateCost;
    float prevCost = node2cost.find(nodeIdx) == node2cost.end() ? cost[nodeIdx] : node2cost[nodeIdx];
    if (realUpdateCost >= prevCost)
        return;
    float deltaCost = prevCost - realUpdateCost;
    if (workloadFreq[nodeIdx])
        reducedCost += (prevCost - realUpdateCost) * float(workloadFreq[nodeIdx]);
    node2cost[nodeIdx] = realUpdateCost;
    const auto &curParentIdx = nodes[nodeIdx].getParentIdx();
    if (curParentIdx.empty())
        return;
    bool curIsEq = nodes[curParentIdx[0]].getIsEq();
    if (curIsEq) {
        for (size_t parentIdx : curParentIdx) {
            float parentPrevCost = node2cost.find(parentIdx) == node2cost.end() ? cost[parentIdx] : node2cost[parentIdx];
            if (realUpdateCost < parentPrevCost)
                updateNodeCost(parentIdx, node2cost, reducedCost, realUpdateCost);
        }
    } else {
        // Parents are op nodes, use cost model to update cost
        for (size_t parentIdx : curParentIdx) {
            char parentOpType = nodes[parentIdx].getOpType();
            float parentPrevCost = node2cost.find(parentIdx) == node2cost.end() ? cost[parentIdx] : node2cost[parentIdx];
            float parentUpdateCost = 0;
            if (parentOpType == 0 || parentOpType == 4)
                updateNodeCost(parentIdx, node2cost, reducedCost, parentPrevCost - deltaCost);
            else if (parentOpType == 1) {
                size_t lChild = 0, rChild = 0;
                const auto &curSiblingIdx = nodes[parentIdx].getChildIdx();
                if (curSiblingIdx[0] == nodeIdx) {
                    lChild = nodeIdx;
                    rChild = curSiblingIdx[1];
                }
                else {
                    lChild = curSiblingIdx[0];
                    rChild = nodeIdx;
                }
                float plan1 = 0, plan2 = 0; // plan1: cost2 + w1 * cost1; plan2: cost1 + w2 * cost2
                float cost1 = node2cost.find(lChild) == node2cost.end() ? cost[lChild] : node2cost[lChild];
                float cost2 = node2cost.find(rChild) == node2cost.end() ? cost[rChild] : node2cost[rChild];
                const auto &lEnd = nodes[lChild].getEndLabel(), &rStart = nodes[rChild].getStartLabel();
                pair<float, float> lrW = getLeftRightWeight(lEnd, rStart);
                float w1 = lrW.first, w2 = lrW.second;
                plan1 = cost2 + w1 * cost1;
                plan2 = cost1 + w2 * cost2;
                parentUpdateCost = plan1 < plan2 ? plan1 : plan2;
                updateNodeCost(parentIdx, node2cost, reducedCost, parentUpdateCost);
            } else if (parentOpType == 2 || parentOpType == 3) {
                pair<float, float> lrW = getLeftRightWeight(nodes[nodeIdx].getEndLabel(), nodes[nodeIdx].getStartLabel());
                float w1 = lrW.first, w2 = lrW.second;
                float curW = w2;
                parentUpdateCost = realUpdateCost;
                size_t curSrcCnt = srcCnt[nodeIdx], curDstCnt = dstCnt[nodeIdx];
                do {
                    parentUpdateCost += curW * realUpdateCost;
                    curSrcCnt *= w1;
                    curDstCnt *= w2;
                    curW *= w2;
                } while (curW >= 0.01 && curSrcCnt + curDstCnt > 0);
                updateNodeCost(parentIdx, node2cost, reducedCost, parentUpdateCost);
            }
        }
    }
}

/**
 * @brief Choose views to materialize given a space budget
 * 
 * @param mode 0: greedy, 1: top workloadFreq, 2: top useCnt, 3: top benefit upper bound (useCnt * (cost - card)),
 * 4: Kleene closures with top benefit upper bound
 * @param spaceBudget space budget (#node pairs, estimated)
 * @param testOut for testing only
 * @return the total real benefit brought by materialization
 */
float AndOrDag::chooseMatViews(char mode, size_t &usedSpace, size_t spaceBudget, std::string *testOut) {
    // Put all candidate views into a priority queue, sorted by benefit (descending)
    usedSpace = 0;
    if (mode == 0) {
        priority_queue<pair<size_t, float>, vector<pair<size_t, float>>, decltype(&PairSecondLess<size_t, float>)> pq(PairSecondLess);
        size_t numNodes = nodes.size();
        for (size_t i = 0; i < numNodes; i++) {
            if (nodes[i].getIsEq() && !nodes[i].getChildIdx().empty()) {
                float benefit = (cost[i] - float(srcCnt[i] * dstCnt[i]) * pairProb[i]) * float(useCnt[i]);
                pq.emplace(i, benefit);
            }
        }
        // Records whether the node's benefit has been computed in the current state
        vector<size_t> benefitComputed(numNodes, 0);
        size_t stateId = 1; // Increment when a new materialized view is selected

        // Pop the element at the top of the heap.
        // If its real benefit has not been computed in the current state, compute.
        //  If the real benefit >0, push it back into the heap. Otherwise, terminate.
        // Otherwise, materialize it.
        // Stopping condition: empty heap, or the top element has real benefit <= 0
        vector<size_t> matIdx;
        unordered_map<size_t, unordered_map<size_t, float>> node2costMap;
        unordered_map<size_t, unordered_map<size_t, float>>::iterator it;
        float realBenefit = 0, totalRealBenefit = 0;
        unordered_map<size_t, float> realBenefitMap;
        size_t addSpace = 0;
        while (usedSpace < spaceBudget && !pq.empty()) {
            size_t curIdx = pq.top().first;
            pq.pop();
            #ifdef TEST
            if (testOut)
                *testOut += to_string(curIdx) + " ";
            #endif
            if (benefitComputed[curIdx] != stateId) {
                matIdx = {curIdx};
                it = node2costMap.find(curIdx);
                if (it == node2costMap.end())
                    node2costMap[curIdx] = unordered_map<size_t, float>();
                else
                    node2costMap[curIdx].clear();
                realBenefit = 0;
                replanWithMaterialize(matIdx, node2costMap[curIdx], realBenefit);
                #ifdef TEST
                if (testOut)
                    *testOut += "1 " + to_string(realBenefit) + " ";
                #endif
                if (realBenefit > 0) {
                    pq.emplace(curIdx, realBenefit);
                    benefitComputed[curIdx] = stateId;
                    realBenefitMap[curIdx] = realBenefit;
                }
                else
                    break;
            } else {
                #ifdef TEST
                if (testOut)
                    *testOut += "0 0 ";
                #endif
                addSpace = float(srcCnt[curIdx] * dstCnt[curIdx]) * pairProb[curIdx];
                if (usedSpace + addSpace > spaceBudget)
                    continue;   // Continue to try other candidates
                materialized[curIdx] = true;
                usedSpace += addSpace;
                applyChanges(node2costMap[curIdx]);
                stateId++;
                totalRealBenefit += realBenefitMap[curIdx];
            }
        }
        return totalRealBenefit;
    } else if (mode == 1 || mode == 2 || mode == 3 || mode == 4) {
        priority_queue<pair<size_t, float>, vector<pair<size_t, float>>, decltype(&PairSecondLess<size_t, float>)> pq(PairSecondLess);
        size_t numNodes = nodes.size();
        for (size_t i = 0; i < numNodes; i++) {
            if (nodes[i].getIsEq() && !nodes[i].getChildIdx().empty()) {
                if (mode == 1)
                    pq.emplace(i, workloadFreq[i]);
                else if (mode == 2)
                    pq.emplace(i, useCnt[i]);
                else if (mode == 3)
                    pq.emplace(i, float(useCnt[i]) * (cost[i] - float(srcCnt[i] * dstCnt[i]) * pairProb[i]));
                else {
                    const auto &curChildIdx = nodes[i].getChildIdx();
                    if (curChildIdx.size() == 1) {
                        char curOpType = nodes[curChildIdx[0]].getOpType();
                        if (curOpType == 2 || curOpType == 3)
                            pq.emplace(i, float(useCnt[i]) * (cost[i] - float(srcCnt[i] * dstCnt[i]) * pairProb[i]));
                    }
                }
            }
        }

        size_t addSpace = 0;
        vector<size_t> matIdx;
        while (usedSpace < spaceBudget && !pq.empty()) {
            size_t curIdx = pq.top().first;
            pq.pop();
            #ifdef TEST
            if (testOut)
                *testOut += to_string(curIdx) + " ";
            #endif
            addSpace = float(srcCnt[curIdx] * dstCnt[curIdx]) * pairProb[curIdx];
            if (usedSpace + addSpace > spaceBudget) {
                #ifdef TEST
                if (testOut)
                    *testOut += "0 ";
                #endif
                continue;
            }
            materialized[curIdx] = true;
            matIdx.emplace_back(curIdx);
            usedSpace += addSpace;
            #ifdef TEST
            if (testOut)
                *testOut += "1 ";
            #endif
        }
        unordered_map<size_t, float> node2cost;
        float realBenefit = 0;
        replanWithMaterialize(matIdx, node2cost, realBenefit);
        applyChanges(node2cost);
        return realBenefit;
    }
    return -1;
}

/**
 * @brief Execute a query with the DAG
 * 
 * @param q the query to execute
 * @param resPtr must pass in nullptr, will be set to the result pointer.
 * Note: since we cannot be sure that the result is new'ed (e.g., base label in CSR), use raw pointer.
 */
void AndOrDag::execute(const std::string &q, QueryResult &qr) {
    if (qr.csrPtr || q.empty())
        return;
    auto it = q2idx.find(q);
    if (it == q2idx.end())
        return;
    size_t qIdx = it->second;
    if (materialized[qIdx])
        qr.csrPtr = &(matRes[qIdx]);    // return pointer to materialized result
    executeNode(qIdx, qr);
}

void AndOrDag::executeNode(size_t nodeIdx, QueryResult &qr, const std::unordered_set<size_t> *lCandPtr,
const std::unordered_set<size_t> *rCandPtr) {
    const auto &curNode = nodes[nodeIdx];
    const auto &curChildIdx = curNode.getChildIdx();
    if (curNode.getIsEq()) {
        if (curChildIdx.empty()) {
            // Single label
            // Implements candidate filtering
            const auto &sl = curNode.getStartLabel()[0];
            double lbl = sl.lbl;
            bool inv = sl.inv;
            auto it = csrPtr->label2idx.find(lbl);
            assert(it != csrPtr->label2idx.end());
            size_t lblIdx = it->second;
            MappedCSR *leftCsrPtr = nullptr, *rightCsrPtr = nullptr;
            // Depending on inv, use variables to handle choice
            if (inv) {
                leftCsrPtr = &(csrPtr->inCsr[lblIdx]);
                rightCsrPtr = &(csrPtr->outCsr[lblIdx]);
            } else {
                leftCsrPtr = &(csrPtr->outCsr[lblIdx]);
                rightCsrPtr = &(csrPtr->inCsr[lblIdx]);
            }
            if (lCandPtr && rCandPtr) {
                // Depending on preference, set the unused one as nullptr
                if (float(lCandPtr->size()) / float(leftCsrPtr->n) < float(rCandPtr->size()) / float(rightCsrPtr->n))   // m is equal
                    rCandPtr = nullptr;
                else
                    lCandPtr = nullptr;
            }

            if (!lCandPtr && !rCandPtr) {
                qr.newed = false;
                qr.csrPtr = leftCsrPtr;
            }
            else {
                qr.csrPtr = new MappedCSR();
                if (lCandPtr && !rCandPtr) {
                    for (size_t curSrc : *lCandPtr) {
                        auto it = leftCsrPtr->v2idx.find(curSrc);
                        if (it != leftCsrPtr->v2idx.end()) {
                            size_t curSrcIdx = it->second;
                            qr.csrPtr->v2idx[curSrc] = qr.csrPtr->offset.size();
                            qr.csrPtr->offset.emplace_back(qr.csrPtr->adj.size());
                            size_t adjStart = leftCsrPtr->offset[curSrcIdx], adjEnd = curSrcIdx < leftCsrPtr->n - 1 ? leftCsrPtr->offset[curSrcIdx + 1] : leftCsrPtr->adj.size();
                            copy(leftCsrPtr->adj.begin() + adjStart, leftCsrPtr->adj.begin() + adjEnd, std::back_inserter(qr.csrPtr->adj));
                        }
                    }
                } else {
                    // Use temporary unordered_map to hold the results
                    unordered_map<size_t, vector<size_t>> tmpNode2Adj;
                    for (size_t curDst : *rCandPtr) {
                        auto it = rightCsrPtr->v2idx.find(curDst);
                        if (it != rightCsrPtr->v2idx.end()) {
                            size_t curDstIdx = it->second;
                            size_t adjStart = rightCsrPtr->offset[curDstIdx], adjEnd = curDstIdx < rightCsrPtr->n - 1 ? rightCsrPtr->offset[curDstIdx + 1] : rightCsrPtr->adj.size();
                            for (size_t i = adjStart; i < adjEnd; i++)
                                tmpNode2Adj[rightCsrPtr->adj[i]].emplace_back(curDst);
                        }
                    }
                    for (const auto &pr : tmpNode2Adj) {
                        size_t curSrc = pr.first;
                        qr.csrPtr->v2idx[curSrc] = qr.csrPtr->offset.size();
                        qr.csrPtr->offset.emplace_back(qr.csrPtr->adj.size());
                        move(pr.second.begin(), pr.second.end(), std::back_inserter(qr.csrPtr->adj));
                    }
                }
                qr.csrPtr->n = qr.csrPtr->v2idx.size();
                qr.csrPtr->m = qr.csrPtr->adj.size();
            }
            
        } else if (curChildIdx.size() == 1) {
            executeNode(curChildIdx[0], qr, lCandPtr, rCandPtr);
            size_t curChildOp = nodes[curChildIdx[0]].getOpType();
            if (curChildOp == 2 || curChildOp == 4)
                qr.hasEpsilon = true;   // Propagate epsilon upwards (* and ?)
        }
        else
            executeNode(curNode.getTargetChild(), qr, lCandPtr, rCandPtr);
    } else {
        char curOpType = curNode.getOpType();
        if (curOpType == 0) {
            // Alternation
            size_t numChild = curChildIdx.size();
            vector<QueryResult> childRes(numChild, {nullptr, false});
            for (size_t i = 0; i < numChild; i++)
                executeNode(curChildIdx[i], childRes[i], lCandPtr, rCandPtr);
            // Combine these results
            qr.csrPtr = new MappedCSR();
            qr.newed = true;
            MappedCSR *curCsrPtr = nullptr, *innerCsrPtr = nullptr;
            for (size_t i = 0; i < numChild; i++) {
                const auto &curGrandchildIdx = nodes[curChildIdx[i]].getChildIdx();
                if (curGrandchildIdx.size() == 1) {
                    char tmpOpType = nodes[curGrandchildIdx[0]].getOpType();
                    if (tmpOpType == 2 || tmpOpType == 4)
                        qr.hasEpsilon = true;   // Propagate epsilon upwards (* and ?)
                }

                curCsrPtr = childRes[i].csrPtr;
                for (const auto &pr : curCsrPtr->v2idx) {
                    size_t v = pr.first, vIdx = pr.second;
                    if (qr.csrPtr->v2idx.find(v) == qr.csrPtr->v2idx.end()) {
                        qr.csrPtr->v2idx[v] = qr.csrPtr->offset.size();
                        qr.csrPtr->offset.emplace_back(qr.csrPtr->adj.size());
                        size_t adjStart = curCsrPtr->offset[vIdx], adjEnd = vIdx < curCsrPtr->n - 1 ? curCsrPtr->offset[vIdx + 1] : curCsrPtr->adj.size();
                        move(curCsrPtr->adj.begin() + adjStart, curCsrPtr->adj.begin() + adjEnd, std::back_inserter(qr.csrPtr->adj));
                        for (size_t j = i + 1; j < numChild; j++) {
                            innerCsrPtr = childRes[j].csrPtr;
                            auto it = innerCsrPtr->v2idx.find(v);
                            if (it != innerCsrPtr->v2idx.end()) {
                                size_t vIdx2 = it->second;
                                size_t adjStart2 = innerCsrPtr->offset[vIdx2], adjEnd2 = vIdx2 < innerCsrPtr->n - 1 ? innerCsrPtr->offset[vIdx2 + 1] : innerCsrPtr->adj.size();
                                move(innerCsrPtr->adj.begin() + adjStart2, innerCsrPtr->adj.begin() + adjEnd2, std::back_inserter(qr.csrPtr->adj));
                            }
                        }
                    }
                }
                if (childRes[i].newed)
                    delete curCsrPtr;
            }
            qr.csrPtr->n = qr.csrPtr->v2idx.size();
            qr.csrPtr->m = qr.csrPtr->adj.size();
        } else if (curOpType == 1) {
            // Concatenation
            // If encounter * or ? type:
            // If left & right both has epsilon, mark as has epsilon;
            // If only left (right) has epsilon, add all the right (left) results into the final result
            QueryResult qrLeft(nullptr, false), qrRight(nullptr, false);
            unordered_set<size_t> curCand;
            if (curNode.getLeft2Right()) {
                executeNode(curChildIdx[0], qrLeft, lCandPtr, nullptr);
                if (!qrLeft.hasEpsilon) {
                    for (size_t x : qrLeft.csrPtr->adj)
                        curCand.emplace(x);
                    executeNode(curChildIdx[1], qrRight, &curCand, nullptr);
                } else
                    executeNode(curChildIdx[1], qrRight, nullptr, nullptr);
            } else {
                executeNode(curChildIdx[1], qrRight, nullptr, rCandPtr);
                if (!qrRight.hasEpsilon) {
                    for (const auto &x : qrRight.csrPtr->v2idx)
                        curCand.emplace(x.first);
                    executeNode(curChildIdx[0], qrLeft, nullptr, &curCand);
                } else
                    executeNode(curChildIdx[0], qrLeft, nullptr, nullptr);
            }
            // Join
            qr.csrPtr = new MappedCSR();
            qr.newed = true;
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
                                qr.csrPtr->adj.emplace_back(nextNextNode);
                            }
                        }
                    }
                }
                if (!qrLeft.hasEpsilon && qrRight.hasEpsilon) {
                    for (size_t i = adjStart; i < adjEnd; i++) {
                        size_t nextNode = qrLeft.csrPtr->adj[i];
                        if (exist.find(nextNode) == exist.end()) {
                            exist.emplace(nextNode);
                            qr.csrPtr->adj.emplace_back(nextNode);
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
                                qr.csrPtr->adj.emplace_back(nextNextNode);
                            }
                        }
                    }
                }
                if (!exist.empty()) {
                    qr.csrPtr->v2idx[v] = qr.csrPtr->offset.size();
                    qr.csrPtr->offset.emplace_back(qr.csrPtr->adj.size() - exist.size());
                }
            }
            if (qrLeft.hasEpsilon && !qrRight.hasEpsilon) {
                // Handle the remaining results on the right
                for (const auto &pr : qrRight.csrPtr->v2idx) {
                    size_t v = pr.first;
                    if (qr.csrPtr->v2idx.find(v) == qr.csrPtr->v2idx.end()) {
                        qr.csrPtr->v2idx[v] = qr.csrPtr->offset.size();
                        qr.csrPtr->offset.emplace_back(qr.csrPtr->adj.size());
                        size_t adjStart = qrRight.csrPtr->offset[pr.second], adjEnd = pr.second < qrRight.csrPtr->n - 1 ? qrRight.csrPtr->offset[pr.second + 1] : qrRight.csrPtr->adj.size();
                        move(qrRight.csrPtr->adj.begin() + adjStart, qrRight.csrPtr->adj.begin() + adjEnd, std::back_inserter(qr.csrPtr->adj));
                    }
                }
            } else if (qrLeft.hasEpsilon && qrRight.hasEpsilon)
                qr.hasEpsilon = true;
            qr.csrPtr->n = qr.csrPtr->v2idx.size();
            qr.csrPtr->m = qr.csrPtr->adj.size();
        } else if (curOpType == 2 || curOpType == 3) {
            QueryResult qrChild(nullptr, false);
            executeNode(curChildIdx[0], qrChild, lCandPtr, rCandPtr);
            unordered_map<size_t, vector<size_t>> node2Adj;
            unordered_map<size_t, size_t> curAdjLen;
            // Fix-point
            for (const auto &pr : qrChild.csrPtr->v2idx) {
                size_t v = pr.first, vIdx = pr.second;
                size_t adjStart = qrChild.csrPtr->offset[vIdx], adjEnd = vIdx < qrChild.csrPtr->n - 1 ? qrChild.csrPtr->offset[vIdx + 1] : qrChild.csrPtr->adj.size();
                unordered_set<size_t> exist(qrChild.csrPtr->adj.begin() + adjStart, qrChild.csrPtr->adj.begin() + adjEnd);
                copy(qrChild.csrPtr->adj.begin() + adjStart, qrChild.csrPtr->adj.begin() + adjEnd, std::back_inserter(node2Adj[v]));
                for (size_t i = adjStart; i < adjEnd; i++) {
                    size_t nextNode = qrChild.csrPtr->adj[i];
                    auto it = qrChild.csrPtr->v2idx.find(nextNode);
                    if (it != qrChild.csrPtr->v2idx.end()) {
                        size_t nextNodeIdx = it->second;
                        size_t adjStart2 = qrChild.csrPtr->offset[nextNodeIdx], adjEnd2 = nextNodeIdx < qrChild.csrPtr->n - 1 ?
                            qrChild.csrPtr->offset[nextNodeIdx + 1] : qrChild.csrPtr->adj.size();
                        for (size_t j = adjStart2; j < adjEnd2; j++) {
                            size_t nextNextNode = qrChild.csrPtr->adj[j];
                            if (exist.find(nextNextNode) == exist.end()) {
                                exist.emplace(nextNextNode);
                                node2Adj[v].emplace_back(nextNextNode);
                            }
                        }
                    }
                }
                if (!exist.empty())
                    curAdjLen[v] = 0;
            }
            size_t numNodesAdded = 0;
            do {
                numNodesAdded = 0;
                for (auto &pr : node2Adj) {
                    size_t v = pr.first, curLen = pr.second.size(), prevLen = curAdjLen[v];
                    if (curLen == prevLen)
                        continue;
                    unordered_set<size_t> exist(pr.second.begin(), pr.second.end());
                    for (size_t i = prevLen; i < curLen; i++) {
                        size_t nextNode = pr.second[i];
                        auto it = qrChild.csrPtr->v2idx.find(nextNode);
                        if (it != qrChild.csrPtr->v2idx.end()) {
                            size_t nextNodeIdx = it->second;
                            size_t adjStart2 = qrChild.csrPtr->offset[nextNodeIdx], adjEnd2 = nextNodeIdx < qrChild.csrPtr->n - 1 ?
                                qrChild.csrPtr->offset[nextNodeIdx + 1] : qrChild.csrPtr->adj.size();
                            for (size_t j = adjStart2; j < adjEnd2; j++) {
                                size_t nextNextNode = qrChild.csrPtr->adj[j];
                                if (exist.find(nextNextNode) == exist.end()) {
                                    exist.emplace(nextNextNode);
                                    pr.second.emplace_back(nextNextNode);
                                    numNodesAdded++;
                                }
                            }
                        }
                    }
                    curAdjLen[v] = curLen;
                }
            } while (numNodesAdded > 0);
            qr.csrPtr = new MappedCSR();
            qr.newed = true;
            for (const auto &pr : node2Adj) {
                size_t v = pr.first;
                qr.csrPtr->v2idx[v] = qr.csrPtr->offset.size();
                qr.csrPtr->offset.emplace_back(qr.csrPtr->adj.size());
                move(pr.second.begin(), pr.second.end(), std::back_inserter(qr.csrPtr->adj));
            }
            qr.csrPtr->n = qr.csrPtr->v2idx.size();
            qr.csrPtr->m = qr.csrPtr->adj.size();
        } else if (curOpType == 4)
            executeNode(curChildIdx[0], qr, lCandPtr, rCandPtr);
    }
}