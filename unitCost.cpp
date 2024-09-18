/**
 * @file unitCost.cpp
 * @author Yue Pang (michelle.py@pku.edu.cn)
 * @brief Implementation of "Estimating searching cost of regular path queries on large graphs by exploiting unit-subqueries."
 * @date 2024-01-11
 */

#include <omp.h>
#include "CSR.h"
#include "Rpq2NFAConvertor.h"
#define MAXTHREAD 4
#define MAXKLEENESTEP 6
#define SAMPLESZ 100

using namespace std;

// Estimate the cost of pathElt[lIdx:rIdx]
double estimateCost(rpqParser::PathContext *path, size_t lIdx, size_t rIdx, std::shared_ptr<MultiLabelCSR> csrPtr) {
    double ret = 0, curCoefficient = 1;
    rpqParser::PathSequenceContext *pathSequence = path->pathSequence()[0];
    double delta = 0, ksai = 0, prevMu = 0;
    bool inverse = false, nextInverse = false, kleene = false, nextKleene = false;
    size_t label = 0, nextLabel = 0, labelIdx = 0, nextLabelIdx = 0, lPos = 0, rPos = 0;
    string pathEltStr;
    unordered_map<double, size_t>::iterator labelIter;
    for (size_t i = lIdx; i < rIdx; i++) {
        // Transition from next to cur
        if (i == lIdx) {
            pathEltStr = pathSequence->pathElt()[i]->getText();
            lPos = pathEltStr.find('<');
            rPos = pathEltStr.find('>');
            inverse = (pathEltStr[rPos - 1] == '-');
            kleene = (pathEltStr[rPos - 1] == '*' || pathEltStr[rPos - 1] == '+');
            label = stoi(pathEltStr.substr(lPos + 1, rPos - lPos - 1 - inverse));
        } else {
            inverse = nextInverse;
            kleene = nextKleene;
            label = nextLabel;
        }
        labelIter = csrPtr->label2idx.find(label);
        if (labelIter == csrPtr->label2idx.end()) {
            cout << "Label " << label << " not found" << endl;
            exit(1);
        }
        labelIdx = labelIter->second;
        delta = csrPtr->outCsr[labelIdx].m;

        // Right edge condition, specially treat i == lIdx
        const MappedCSR *lblCsrPtr = nullptr, *tmpLblCsrPtr = nullptr;
        AdjInterval aitv;
        size_t numOutLabel = csrPtr->outCsr.size(), numInLabel = csrPtr->inCsr.size();
        if (!inverse)
            lblCsrPtr = &(csrPtr->inCsr[labelIdx]); // targets of the current label
        else
            lblCsrPtr = &(csrPtr->outCsr[labelIdx]);    // sources of the current label
        size_t numNodes = lblCsrPtr->n;
        unordered_map<unsigned, unsigned>::const_iterator v2idxIt;
        double curMu = 0, kleeneMu = 0; // Only estimate kleeneMu if kleene == true
        if (kleene) {
            if (SAMPLESZ >= numNodes) {
                // Accurately collate
                // How to get a node's degree? (API) getAdjIntervalByVert. How about total degree regardless of label? Scan all the CSRs
                if (!inverse) {
                    for (const auto &pr : lblCsrPtr->v2idx) {
                        size_t v = pr.first;
                        for (size_t j = 0; j < numOutLabel; j++) {
                            tmpLblCsrPtr = &(csrPtr->outCsr[j]);
                            tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                            if (kleene && labelIdx == j)
                                kleeneMu += aitv.len;
                        }
                    }
                } else {
                    for (const auto &pr : lblCsrPtr->v2idx) {
                        size_t v = pr.first;
                        for (size_t j = 0; j < numInLabel; j++) {
                            tmpLblCsrPtr = &(csrPtr->inCsr[j]);
                            tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                            if (kleene && labelIdx == j)
                                kleeneMu += aitv.len;
                        }
                    }
                }
            } else {
                // Do sampling
                if (!inverse) {
                    for (size_t k = 0; k < SAMPLESZ; k++) {
                        size_t curIdx = rand() % numNodes;
                        v2idxIt = lblCsrPtr->v2idx.begin();
                        std::advance(v2idxIt, curIdx);
                        size_t v = v2idxIt->first;
                        for (size_t j = 0; j < numOutLabel; j++) {
                            tmpLblCsrPtr = &(csrPtr->outCsr[j]);
                            tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                            if (kleene && labelIdx == j)
                                kleeneMu += aitv.len;
                        }
                    }
                } else {
                    for (size_t k = 0; k < SAMPLESZ; k++) {
                        size_t curIdx = rand() % numNodes;
                        v2idxIt = lblCsrPtr->v2idx.begin();
                        std::advance(v2idxIt, curIdx);
                        size_t v = v2idxIt->first;
                        for (size_t j = 0; j < numOutLabel; j++) {
                            tmpLblCsrPtr = &(csrPtr->inCsr[j]);
                            tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                            if (kleene && labelIdx == j)
                                kleeneMu += aitv.len;
                        }
                    }
                }
            }
        }
        if (i == lIdx) {
            // Do not need to collate ksai, cost == cardinality
            if (kleene) {
                double omega = kleeneMu / delta;
                double multiplier = 1;
                for (size_t i = 0; i < MAXKLEENESTEP; i++)
                    multiplier *= omega;
                multiplier = (multiplier - 1) / (omega - 1);
                curCoefficient *= multiplier;
                ret += curCoefficient * delta;  // Note: multiplying ksai is not possible for a single Kleene closure
            } else
                ret += delta;
        }

        if (i < rIdx - 1) {
            pathEltStr = pathSequence->pathElt()[i+1]->getText();
            lPos = pathEltStr.find('<');
            rPos = pathEltStr.find('>');
            nextInverse = (pathEltStr[rPos - 1] == '-');
            nextKleene = (pathEltStr[rPos - 1] == '*' || pathEltStr[rPos - 1] == '+');
            nextLabel = stoi(pathEltStr.substr(lPos + 1, rPos - lPos - 1 - inverse));
            labelIter = csrPtr->label2idx.find(nextLabel);
            if (labelIter == csrPtr->label2idx.end()) {
                cout << "nextLabel " << nextLabel << " not found" << endl;
                exit(1);
            }
            nextLabelIdx = labelIter->second;
        }

        // Estimate ksai and curMu by Monte Carlo sampling
        // if the current label is a_k, then curMu is mu(a_k, a_{k+1})
        ksai = 0;
        if (SAMPLESZ >= numNodes) {
            // Accurately collate
            // How to get a node's degree? (API) getAdjIntervalByVert. How about total degree regardless of label? Scan all the CSRs
            if (!nextInverse) {
                for (const auto &pr : lblCsrPtr->v2idx) {
                    size_t v = pr.first;
                    for (size_t j = 0; j < numOutLabel; j++) {
                        tmpLblCsrPtr = &(csrPtr->outCsr[j]);
                        tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                        ksai += aitv.len;
                        if (i < rIdx - 1 && nextLabelIdx == j)
                            curMu += aitv.len;
                    }
                }
            } else {
                for (const auto &pr : lblCsrPtr->v2idx) {
                    size_t v = pr.first;
                    for (size_t j = 0; j < numInLabel; j++) {
                        tmpLblCsrPtr = &(csrPtr->inCsr[j]);
                        tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                        ksai += aitv.len;
                        if (i < rIdx - 1 && nextLabelIdx == j)
                            curMu += aitv.len;
                    }
                }
            }
        } else {
            // Do sampling
            if (!nextInverse) {
                for (size_t k = 0; k < SAMPLESZ; k++) {
                    size_t curIdx = rand() % numNodes;
                    v2idxIt = lblCsrPtr->v2idx.begin();
                    std::advance(v2idxIt, curIdx);
                    size_t v = v2idxIt->first;
                    for (size_t j = 0; j < numOutLabel; j++) {
                        tmpLblCsrPtr = &(csrPtr->outCsr[j]);
                        tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                        ksai += aitv.len;
                        if (i < rIdx - 1 && nextLabelIdx == j)
                            curMu += aitv.len;
                    }
                }
            } else {
                for (size_t k = 0; k < SAMPLESZ; k++) {
                    size_t curIdx = rand() % numNodes;
                    v2idxIt = lblCsrPtr->v2idx.begin();
                    std::advance(v2idxIt, curIdx);
                    size_t v = v2idxIt->first;
                    for (size_t j = 0; j < numOutLabel; j++) {
                        tmpLblCsrPtr = &(csrPtr->inCsr[j]);
                        tmpLblCsrPtr->getAdjIntervalByVert(v, aitv);
                        ksai += aitv.len;
                        if (i < rIdx - 1 && nextLabelIdx == j)
                            curMu += aitv.len;
                    }
                }
            }
        }

        if (!kleene) {
            if (i == lIdx)
                ret += ksai;
            else {
                curCoefficient *= prevMu / delta;
                ret += curCoefficient * ksai;
            }
        } else if (i != lIdx) {
            // Handle Kleene closures
            double omega = kleeneMu / delta;
            double multiplier = 1;
            for (size_t i = 0; i < MAXKLEENESTEP; i++)
                multiplier *= omega;
            multiplier = (multiplier - 1) / (omega - 1);
            curCoefficient *= multiplier;
            ret += curCoefficient * ksai;
        }
        prevMu = curMu;
    }
    return ret;
}

void getBestSplit(rpqParser::PathContext *path, vector<string> &splitParts, std::shared_ptr<MultiLabelCSR> csrPtr) {
    rpqParser::PathSequenceContext *pathSequence = path->pathSequence()[0];
    size_t maxThreadNum = pathSequence->pathElt().size() - 1;
    if (maxThreadNum > MAXTHREAD) maxThreadNum = MAXTHREAD;
    // Note: there are many opportunities for shared computation here, but we do not implement them for simplicity
    vector<string> curSplitParts;
    double minCost = LLONG_MAX;
    for (size_t curThreadNum = 2; curThreadNum <= maxThreadNum; curThreadNum++) {
        // Get all possible sequences of (curThreadNum-1) split points
        vector<size_t> splitPoints(curThreadNum - 1, 0);
        for (size_t i = 1; i < curThreadNum - 1; i++)
            splitPoints[i] = i;
        do {
            double localMaxCost = 0;
            for (size_t i = 0; i < curThreadNum; i++) {
                size_t lIdx = i == 0 ? 0 : splitPoints[i-1] + 1;
                size_t rIdx = i == curThreadNum - 1 ? pathSequence->pathElt().size() : splitPoints[i] + 1;
                localMaxCost = max(localMaxCost, estimateCost(path, lIdx, rIdx, csrPtr));   // Cost verified
            }
            if (localMaxCost < minCost) {
                minCost = localMaxCost;
                curSplitParts.clear();
                for (size_t i = 0; i < curThreadNum; i++) {
                    size_t lIdx = i == 0 ? 0 : splitPoints[i-1] + 1;
                    size_t rIdx = i == curThreadNum - 1 ? pathSequence->pathElt().size() : splitPoints[i] + 1;
                    string curSplitPart;
                    for (size_t j = lIdx; j < rIdx; j++) {
                        if (!curSplitPart.empty())
                            curSplitPart += "/";
                        curSplitPart += pathSequence->pathElt()[j]->getText();
                    }
                    curSplitParts.emplace_back(curSplitPart);
                }
            }
            // Get next split point sequence (verified)
            int curIdx = curThreadNum - 2;
            bool terminate = false;
            while (splitPoints[curIdx] == pathSequence->pathElt().size() - (curThreadNum - curIdx)) {
                curIdx--;
                if (curIdx == -1) {
                    terminate = true;
                    break;
                }
            }
            if (terminate)
                break;
            splitPoints[curIdx]++;
            for (size_t i = curIdx + 1; i < curThreadNum - 1; i++)
                splitPoints[i] = splitPoints[i-1] + 1;
        } while (true);
    }
    splitParts.swap(curSplitParts);
}

string removeRedundantParen(rpqParser::PathContext *path) {
    string ret;
    size_t pathSequenceNum = path->pathSequence().size();
    for (size_t i = 0; i < pathSequenceNum; i++) {
        if (i > 0)
            ret += "|";
        rpqParser::PathSequenceContext *pathSequence = path->pathSequence()[i];
        size_t pathEltNum = pathSequence->pathElt().size();
        for (size_t j = 0; j < pathEltNum; j++) {
            if (j > 0)
                ret += "/";
            rpqParser::PathEltContext *pathElt = pathSequence->pathElt()[j];
            if (pathElt->pathPrimary()->path()) {
                // If no pathMod and the lowest-precedence op is /, safe to remove parentheses
                if (!pathElt->pathMod() && pathElt->pathPrimary()->path()->pathSequence().size() == 1)
                    ret += removeRedundantParen(pathElt->pathPrimary()->path());
                else {
                    ret += "(" + removeRedundantParen(pathElt->pathPrimary()->path()) + ")";
                    if (pathElt->pathMod())
                        ret += pathElt->pathMod()->getText();
                }
            } else
                ret += pathElt->getText();
        }
    }
    return ret;
}

int main() {
    // Read workload queries
    string dataDir = "../real_data/";
    string graphName = "wikidata";
    string queryFilePath = dataDir + graphName + "/queries.txt";
    ifstream fin(queryFilePath);
    unordered_set<string> qSet;
    unordered_map<string, vector<string>> qMap;
    string line, q;
    while (fin >> q)
        qSet.emplace(q);

    // Remove redundant parentheses
    for (const string &query : qSet) {
        istringstream ifs(query);
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
        string cleanQuery = removeRedundantParen(path);
        qMap[cleanQuery] = vector<string>();
    }

    // Identify cases that cannot be decomposed: the whole query is a Kleene closure,
    // or includes a Kleene closure that is not a single label's
    for (auto &p: qMap) {
        string query = p.first;
        istringstream ifs(query);
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
        if (path->pathSequence().size() == 1 && path->pathSequence()[0]->pathElt().size() == 1) {
            rpqParser::PathEltContext *pathElt = path->pathSequence()[0]->pathElt()[0];
            if (pathElt->pathMod()) {
                string pathModStr = pathElt->pathMod()->getText();
                if (pathModStr == "*" || pathModStr == "+") {
                    cout << "Cannot decompose: " << query << endl;
                    continue;
                }
            }
        } else {
            bool cannotDecompose = false;
            for (const auto &pathSequence: path->pathSequence()) {
                for (const auto &pathElt : pathSequence->pathElt()) {
                    if (pathElt->pathPrimary()->path() && pathElt->pathMod()) {
                        rpqParser::PathContext *innerPath = pathElt->pathPrimary()->path();
                        if (innerPath->pathSequence().size() == 1 && innerPath->pathSequence()[0]->pathElt().size() == 1
                            && !innerPath->pathSequence()[0]->pathElt()[0]->pathMod() && innerPath->pathSequence()[0]->pathElt()[0]->pathPrimary()->iri())
                            continue;
                        string pathModStr = pathElt->pathMod()->getText();
                        if (pathModStr == "*" || pathModStr == "+") {
                            cout << "Cannot decompose: " << query << endl;
                            cannotDecompose = true;
                            break;
                        }
                    }
                }
                if (cannotDecompose)
                    break;
            }
            if (cannotDecompose)
                continue;
        }
        for (const auto &pathSequence: path->pathSequence())
            p.second.emplace_back(pathSequence->getText());
    }

    // Decompose workload queries
    // Assume only a layer of decomposition
    for (auto &p: qMap) {
        vector<string> decomposed;
        std::move(p.second.begin(), p.second.end(), std::back_inserter(decomposed));
        p.second.clear();
        for (size_t k = 0; k < decomposed.size(); k++) {
            const auto &pathStr = decomposed[k];
            istringstream ifs(pathStr);
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
            if (path->pathSequence().size() > 1) {
                for (const auto &pathSequence: path->pathSequence())
                    decomposed.emplace_back(pathSequence->getText());
                continue;
            }
            const auto &pathEltVec = path->pathSequence()[0]->pathElt();
            bool cannotDecompose = true;
            for (size_t i = 0; i < pathEltVec.size(); i++) {
                rpqParser::PathContext *nextPath = pathEltVec[i]->pathPrimary()->path();
                if (nextPath && nextPath->pathSequence().size() > 1) {
                    // Concat the left and right parts with different clauses
                    string left, right;
                    for (size_t j = 0; j < i; j++)
                        left += pathEltVec[j]->getText() + "/";
                    for (size_t j = i + 1; j < pathEltVec.size(); j++)
                        right += "/" + pathEltVec[j]->getText();
                    for (const auto &nextPathSequence: nextPath->pathSequence())
                        decomposed.emplace_back(left + nextPathSequence->getText() + right);
                    cannotDecompose = false;
                    break;
                }
            }
            if (cannotDecompose)
                p.second.emplace_back(pathStr);
        }
    }

    // Test effects here
    for (auto &p: qMap) {
        cout << p.first << " : ";
        for (const auto &pathStr: p.second)
            cout << pathStr << " ";
        cout << endl;
    }

    // Read graph
    std::shared_ptr<MultiLabelCSR> csrPtr = make_shared<MultiLabelCSR>();
    string graphFilePath = dataDir + graphName + "/graph.txt";
    LineSeq lseq = sop;
    if (graphName == "wikidata")
        lseq = spo;
    auto start_time = std::chrono::steady_clock::now();
    csrPtr->loadGraph(graphFilePath, lseq);
    // csrPtr->fillStats();
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::microseconds elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Read graph time: " << elapsed_microseconds.count() / 1000.0 << " ms" << std::endl;

    // For each distinct workload query
    // - if cannot decompose, do not execute, output -1 (directly use the previous time) [DONE]
    // - else, for each decomposed subquery, (Note: do not explicitly implement alternation)
    // [Do multi-thread here? Needs OpenMP nested parallelism, may not be beneficial, consider later]
    //  - if k (k > 2) pathElts, enumerate all splitting possibilities with 2, 3, ..., k-1 threads,
    //  choose the one whose split subquery with the max cost is the smallest, then execute in parallel
    //  Finally, join with a single thread.
    //  Join API: QueryResult::assignAsJoin; naive method outputs shared_ptr<MappedCSR>, set QueryResult's ptr as its get(),
    //  set newed as false, delete the object by calling reset(nullptr) on the shared_ptr
    //  - else, use naive method to execute
    // Collate the execution & planning+execution time separately
    Rpq2NFAConvertor cvrt;
    std::chrono::microseconds subExeTime, planExeTime;
    std::chrono::_V2::steady_clock::time_point inner_start_time, inner_end_time;
    long long exeTime = 0;
    for (const auto &p: qMap) {
        if (p.second.empty())
            cout << p.first << " -1 -1" << endl;
        else {
            exeTime = 0;
            start_time = std::chrono::steady_clock::now();
            for (const auto &subquery: p.second) {
                istringstream ifs(subquery);
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
                rpqParser::PathSequenceContext *pathSequence = path->pathSequence()[0];

                if (pathSequence->pathElt().size() > 2) {
                    // Planning:
                    // Single-label cost = #edges
                    // Only 3 cases: concatenation of single labels, single-label-Kleene closures, and single-label "?"
                    // Treat single-label "?" in the same way as single labels
                    // Distinguish between labels and label inverses
                    vector<string> splitParts;
                    getBestSplit(path, splitParts, csrPtr);
                    size_t numSplitParts = splitParts.size();
                    vector<shared_ptr<MappedCSR>> resPtrVec(numSplitParts, nullptr);

                    inner_start_time = std::chrono::steady_clock::now();
                    // Multi-thread execution
                    #pragma omp parallel for
                    for (size_t i = 0; i < numSplitParts; i++) {
                        shared_ptr<NFA> dfaPtr = cvrt.convert(splitParts[i])->convert2Dfa();
                        resPtrVec[i] = dfaPtr->execute(csrPtr);
                    }
                    // Single-thread join
                    // two at a time (i and i+1), put the result into i+1 slot. Final result: resPtrVec[numSplitParts-1]
                    for (size_t i = 0; i < numSplitParts - 1; i++) {
                        QueryResult lQr(resPtrVec[i].get(), false), rQr(resPtrVec[i+1].get(), false), outQr(nullptr, false);
                        outQr.assignAsJoin(lQr, rQr);
                        resPtrVec[i+1].reset(outQr.csrPtr);
                    }
                    inner_end_time = std::chrono::steady_clock::now();
                    subExeTime = std::chrono::duration_cast<std::chrono::microseconds>(inner_end_time - inner_start_time);
                } else {
                    // Naive execution
                    inner_start_time = std::chrono::steady_clock::now();
                    shared_ptr<NFA> dfaPtr = cvrt.convert(subquery)->convert2Dfa();
                    shared_ptr<MappedCSR> res = dfaPtr->execute(csrPtr);
                    inner_end_time = std::chrono::steady_clock::now();
                    subExeTime = std::chrono::duration_cast<std::chrono::microseconds>(inner_end_time - inner_start_time);
                }
                exeTime += subExeTime.count();
            }
            end_time = std::chrono::steady_clock::now();
            planExeTime = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            cout << p.first << ' ' << exeTime << ' ' << planExeTime.count() << endl;
        }
    }
}