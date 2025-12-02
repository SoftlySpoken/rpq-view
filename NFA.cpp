/**
 * @file NFA.cpp
 * @author Yue Pang 
 * @brief Implements methods in NFA.h
 * @date 2022-08-24
 */

#include "NFA.h"

using namespace std;

Transition::Transition(): lbl(-1), forward(true), dst(nullptr) {}

Transition::Transition(int lbl_, bool forward_, std::shared_ptr<State> dst_):
    lbl(lbl_), forward(forward_), dst(dst_) {}

/**
 * @brief Add a transition as described by the arguments.
 * 
 * @param lbl_ 
 * @param forward_ 
 * @param dst_ 
 */
void State::addTransition(int lbl_, bool forward_, std::shared_ptr<State> dst_)
{
    outEdges.emplace_back(Transition(lbl_, forward_, dst_));
}

/**
 * @brief Add a transition as dscribed by the argument.
 * 
 * @param tr the sample transition to copy
 */
void State::addTransition(const Transition &tr)
{
    outEdges.emplace_back(tr);
}

/**
 * @brief Print the info of the current state, the states it can reach,
 * and the transitions between them.
 */
void State::print()
{
    for (auto outEdge : outEdges)
        cout << id << "-[" << outEdge.lbl << (outEdge.forward ? "" : "-") \
            << "]->" << (outEdge.dst)->id << endl;
}

/**
 * @brief Add a state and return the pointer to it
 * 
 * @return std::shared_ptr<State> pointer to the added state
 */
std::shared_ptr<State> NFA::addState(bool accept_ = false)
{
    shared_ptr<State> tmp = make_shared<State>(curMaxId++, accept_);
    states.emplace_back(tmp);
    return tmp;
}

void NFA::addStates(std::vector<std::shared_ptr<State>> someStates)
{
    if (someStates.empty())
        return;
    size_t oriNum = states.size();
    copy(someStates.begin(), someStates.end(), back_inserter(states));
    for (size_t i = oriNum; i < states.size(); i++)
        states[i]->id = curMaxId++;
}

/**
 * @brief Return the state with the given id
 * 
 * @param id_ the given id
 * @return std::shared_ptr<State> pointer to the state with the given id
 */
std::shared_ptr<State> NFA::id2state(int id_)
{
    for (auto someState : states)
        if (someState->id == id_)
            return someState;
    return nullptr;
}

/**
 * @brief Return the state with the given idSet
 * 
 * @param id_ the given idSet
 * @return std::shared_ptr<State> pointer to the state with the given idSet
 */
std::shared_ptr<State> NFA::idSet2state(std::unordered_set<int> idSet_)
{
    for (auto someState : states)
        if (someState->idSet == idSet_)
            return someState;
    return nullptr;
}

/**
 * @brief Set a state as accept state
 * 
 * @param someState the state to set
 */
void NFA::setAccept(std::shared_ptr<State> someState)
{
    someState->accept = true;
    if (find(accepts.begin(), accepts.end(), someState) == accepts.end())
        accepts.emplace_back(someState);
}

/**
 * @brief Set some states as accept states
 * 
 * @param someStates the states to set
 */
void NFA::setAccept(std::vector<std::shared_ptr<State>> someStates)
{
    if (someStates.empty())
        return;
    for (auto someState : someStates)
        someState->accept = true;
    copy(someStates.begin(), someStates.end(), back_inserter(accepts));
}

/**
 * @brief Remove a state from accept states
 * 
 * @param someState the state to remove
 */
void NFA::unsetAccept(std::shared_ptr<State> someState)
{
    someState->accept = false;
    auto pos = find(accepts.begin(), accepts.end(), someState);
    if (pos != accepts.end())
        accepts.erase(pos);
}

/**
 * @brief Reset all currently accept states
 * 
 */
void NFA::unsetAccept()
{
    for (auto someState: accepts)
        someState->accept = false;
    accepts.clear();
}

void NFA::print()
{
    cout << "Initial state: " << initial->id << endl;
    cout << "Accept states: ";
    for (auto accept : accepts)
        cout << accept->id << ' ';
    cout << endl;
    for (auto someState : states)
    {
        cout << someState->id;
        if (!someState->idSet.empty())
        {
            cout << '(';
            for (auto id_ : someState->idSet)
                cout << id_ << ',';
            cout << ')';
        }
        cout << ' ';
    }
    cout << endl;
    for (auto s : states)
        s->print();
}

/**
 * @brief Convert NFA to DFA and return the pointer to it
 * 
 * @return std::shared_ptr<NFA> pointer to the DFA
 */
std::shared_ptr<NFA> NFA::convert2Dfa()
{
    unordered_map<int, unordered_set<int>> closures;
    findEpsClosure(closures);

    shared_ptr<NFA> ret = make_shared<NFA>();
    ret->initial->idSet = closures[this->initial->id];
    ret->unsetAccept();
    vector<bool> used(closures.size(), false);  // Whether an NFA state's closure is a DFA state
    used[this->initial->id] = true;
    for (auto i : closures[this->initial->id])
        if (this->isAccept(this->id2state(i))) {
            ret->setAccept(ret->initial);
            break;
        }
    
    // BFS
    vector<int> q{this->initial->id};
    size_t currIdx = 0;
    while (currIdx < q.size())
    {
        auto currStateDfa = ret->idSet2state(closures[q[currIdx]]);
        if (currStateDfa == nullptr)
        {
            currIdx++;
            continue;
        }
        for (auto currId : closures[q[currIdx]])
        {
            auto currState = id2state(currId);
            for (auto outEdge : currState->outEdges)
            {
                if (outEdge.lbl == -1)
                    continue;
                int outId = outEdge.dst->id;
                if (!used[outId])
                {
                    auto addedState = ret->addState();
                    addedState->idSet = closures[outId];
                    for (int x : closures[outId]) {
                        if (id2state(x)->accept) {
                            ret->setAccept(addedState);
                            break;
                        }
                    }
                    currStateDfa->addTransition(outEdge.lbl, outEdge.forward, addedState);
                    q.emplace_back(outId);
                    used[outId] = true;
                }
                else
                    currStateDfa->addTransition(outEdge.lbl, outEdge.forward, 
                        ret->idSet2state(closures[outId]));
            }
        }
        currIdx++;
    }
    return ret;
}

/**
 * @brief Find the epsilon closures of all states in NFA
 * 
 * @param closures output the mapping from state to epsilon closure
 * @param vis states whose epsilon closure has been computed
 * @param someState the current state
 */
void NFA::findEpsClosure(std::unordered_map<int, std::unordered_set<int>> &closures)
{
    vector<int> vis;
    for (const auto &someState : states)
    {
        vis.assign(curMaxId, false);
        vis[someState->id] = true;
        closures[someState->id].insert(someState->id);

        size_t currIdx = 0;
        vector<shared_ptr<State>> q{someState};
        while (currIdx < q.size())
        {
            auto currState = q[currIdx];
            for (auto outEdge : currState->outEdges)
            {
                if (outEdge.lbl == -1 && !vis[(outEdge.dst)->id])
                {
                    closures[someState->id].insert((outEdge.dst)->id);
                    q.emplace_back(outEdge.dst);
                    vis[(outEdge.dst)->id] = true;
                }
            }
            currIdx++;
        }
    }

    // for (auto pr : closures)
    // {
    //     cout << '[' << pr.first << "] ";
    //     for (auto id_ : pr.second)
    //         cout << id_ << ' ';
    //     cout << endl;
    // }
    return;
}

void NFA::reverse() {
    // Reverse all edges' directions
    size_t numState = states.size();
    vector<size_t> numOriEdge(numState, 0);
    for (size_t i = 0; i < numState; i++)
        numOriEdge[i] = (states[i]->outEdges).size();
    for (size_t i = 0; i < numState; i++) {
        for (size_t j = 0; j < numOriEdge[i]; j++) {
            auto &oe = states[i]->outEdges[j];
            (oe.dst)->addTransition(oe.lbl, !(oe.forward), states[i]);
        }
        states[i]->outEdges.assign(states[i]->outEdges.begin() + numOriEdge[i], states[i]->outEdges.end());
    }
    
    // Add synthetic initial state
    auto initialNew = addState(false);
    for (const auto &s : accepts)
        initialNew->addTransition(-1, true, s);

    // Switch initial & accept states
    unsetAccept();
    setAccept(initial);
    initial = initialNew;
}

// DFS execution, return true as soon as a result is found
bool NFA::checkIfValidSrc(size_t dataNode, std::shared_ptr<const MultiLabelCSR> csrPtr, int curVisMark) {
    stack<pair<unsigned, shared_ptr<State>>> st;
    shared_ptr<State> s0 = this->initial;
    unsigned v, nextV;
    shared_ptr<State> s;
    AdjInterval aitv;
    st.emplace(dataNode, s0);
    pair<unsigned, shared_ptr<State>> pr;
    unordered_map<double, size_t>::const_iterator it;
    while (!st.empty()) {
        pr = st.top();
        st.pop();
        v = pr.first;
        s = pr.second;
        // Early return true when the next state is accept
        for (const auto &oe : s->outEdges) {
            it = csrPtr->label2idx.find(oe.lbl);
            if (it == csrPtr->label2idx.end())
                continue;
            size_t curLblIdx = it->second;
            auto forward = oe.forward;
            auto dst = oe.dst;
            if (forward) {
                csrPtr->outCsr[curLblIdx].getAdjIntervalByVert(v, aitv);
                if (aitv.len > 0) {
                    if (this->isAccept(dst))
                        return true;
                    for (size_t j = 0; j < aitv.len; j++) {
                        nextV = (*aitv.start)[aitv.offset + j];
                        if (vis[dst->id][nextV] != curVisMark) {
                            // cout << v << ',' << nextV << ',' << dst->id << ' ';
                            st.emplace(nextV, dst);
                            vis[dst->id][nextV] = curVisMark;
                        }
                    }
                }
            } else {
                csrPtr->inCsr[curLblIdx].getAdjIntervalByVert(v, aitv);
                if (aitv.len > 0) {
                    if (this->isAccept(dst))
                        return true;
                    for (size_t j = 0; j < aitv.len; j++) {
                        nextV = (*aitv.start)[aitv.offset + j];
                        if (vis[dst->id][nextV] != curVisMark) {
                            // cout << v << ',' << nextV << ',' << dst->id << ' ';
                            st.emplace(nextV, dst);
                            vis[dst->id][nextV] = curVisMark;
                        }
                    }
                }
            }
        }
    }
    return false;
}

std::shared_ptr<MappedCSR> NFA::execute(std::shared_ptr<const MultiLabelCSR> csrPtr) {
    queue<pair<unsigned, shared_ptr<State>>> q;
    shared_ptr<State> s0 = this->initial;
    unsigned v, nextV;
    shared_ptr<State> s;
    AdjInterval aitv;
    size_t prevSz;
    vector<unsigned> tmpAdj, tmpOffset;
    shared_ptr<MappedCSR> ret = make_shared<MappedCSR>();
    unordered_set<unsigned> src;
    unsigned sNode = 0;
    clearVis(csrPtr->maxNode + 1);
    for (const auto &initOut : s0->outEdges) {
        auto it = csrPtr->label2idx.find(initOut.lbl);
        assert(it != csrPtr->label2idx.end());
        size_t lblIdx = it->second;
        const unordered_map<unsigned int, unsigned int> *v2idxPtr = &(csrPtr->outCsr[lblIdx].v2idx);
        if (!initOut.forward)
            v2idxPtr = &(csrPtr->inCsr[lblIdx].v2idx);
        for (const auto &spr : *v2idxPtr) {
            sNode = spr.first;
            if (src.find(sNode) != src.end())
                continue;
            src.emplace(sNode);
            // Skip clearVis, assuming n does not exceed INT_MAX
            prevSz = tmpAdj.size();
            q.push(make_pair(sNode, s0));
            vis[s0->id][sNode] = sNode;
            while (!q.empty()) {
                auto cur = q.front();
                q.pop();
                v = cur.first;
                s = cur.second;
                if (this->isAccept(s))
                    tmpAdj.emplace_back(v);
                for (const auto &oe : s->outEdges) {
                    it = csrPtr->label2idx.find(oe.lbl);
                    if (it == csrPtr->label2idx.end())
                        continue;
                    size_t curLblIdx = it->second;
                    auto forward = oe.forward;
                    auto dst = oe.dst;
                    if (forward) {
                        csrPtr->outCsr[curLblIdx].getAdjIntervalByVert(v, aitv);
                        if (aitv.len > 0) {
                            for (size_t j = 0; j < aitv.len; j++) {
                                nextV = (*aitv.start)[aitv.offset + j];
                                if (vis[dst->id][nextV] != int(sNode)) {
                                    // cout << v << ',' << nextV << ',' << dst->id << ' ';
                                    q.push(make_pair(nextV, dst));
                                    vis[dst->id][nextV] = sNode;
                                }
                            }
                        }
                    } else {
                        csrPtr->inCsr[curLblIdx].getAdjIntervalByVert(v, aitv);
                        if (aitv.len > 0) {
                            for (size_t j = 0; j < aitv.len; j++) {
                                nextV = (*aitv.start)[aitv.offset + j];
                                if (vis[dst->id][nextV] != int(sNode)) {
                                    // cout << v << ',' << nextV << ',' << dst->id << ' ';
                                    q.push(make_pair(nextV, dst));
                                    vis[dst->id][nextV] = sNode;
                                }
                            }
                        }
                    }
                }
            }
            if (tmpAdj.size() > prevSz) {
                ret->v2idx[sNode] = tmpOffset.size();
                tmpOffset.emplace_back(prevSz);
            }
        }
    }
    ret->n = tmpOffset.size();
    ret->offset = move(tmpOffset);
    ret->m = tmpAdj.size();
    ret->adj = move(tmpAdj);
    return ret;
}

void NFA::clearVis(unsigned gN) {
    size_t numStates = states.size();
    if (!vis) {
        vis = new int *[numStates];
        for (size_t j = 0; j < numStates; j++) {
            vis[j] = new int [gN];
            memset(vis[j], -1, gN * sizeof(int));
        }
    } else {
        for (size_t j = 0; j < numStates; j++)
            memset(vis[j], -1, gN * sizeof(int));
    }
}

// Helper function to add a pair to the map if not exists (return true if added, false if already exists), and return the id as output param
bool checkAddPair(size_t v1, size_t v2, unordered_map<size_t, unordered_map<size_t, size_t>> &ump, size_t &curMaxId, size_t &id) {
    bool ret = true;
    auto it = ump.find(v1);
    if (it == ump.end()) {
        ump[v1] = unordered_map<size_t, size_t>();
        id = curMaxId;
        ump[v1][v2] = curMaxId++;
    } else {
        auto it2 = it->second.find(v2);
        if (it2 == it->second.end()) {
            id = curMaxId;
            ump[v1][v2] = curMaxId++;
        } else {
            id = it2->second;
            ret = false;
        }
    }
    return ret;
}

LineGraph LineGraph::operator * (const LineGraph &lg) const {
    LineGraph prod;

    size_t numNodesL = this->nodeLabel.size(), numNodesR = lg.nodeLabel.size();
    size_t numNodesProd = 0;
    unordered_map<size_t, unordered_map<size_t, size_t>> pair2prodNode;
    for (size_t v1 = 0; v1 < numNodesL; v1++) {
        for (size_t v2 = 0; v2 < numNodesR; v2++) {
            if (this->nodeLabel[v1] != lg.nodeLabel[v2])
                continue;
            for (size_t jl = 0; jl < this->outAdj[v1].size(); jl++) {
                for (size_t jr = 0; jr < lg.outAdj[v2].size(); jr++) {
                    size_t u1 = this->outAdj[v1][jl], u2 = lg.outAdj[v2][jr];
                    if (this->outLabel[v1][jl] != lg.outLabel[v2][jr] || this->nodeLabel[u1] != lg.nodeLabel[u2])
                       continue;
                    // Nodes will not be added if no edge
                    size_t srcId = 0, dstId = 0;
                    bool added = false;
                    added = checkAddPair(v1, v2, pair2prodNode, numNodesProd, srcId);
                    if (added)
                        prod.addNode(this->nodeLabel[v1]);
                    added = checkAddPair(u1, u2, pair2prodNode, numNodesProd, dstId);
                    if (added)
                        prod.addNode(this->nodeLabel[u1]);
                    prod.addEdge(srcId, dstId, this->outLabel[v1][jl]);
                }
            }
        }
    }
    if (prod.nodeLabel.empty() || prod.outAdj.empty())
        return prod;    // Early stop, skip the weak connections part

    return prod;
}

// 0: no nodes / no edges / no non-weak edges, 1: clique, 2: otherwise
int LineGraph::getType() const {
    if (nodeLabel.empty() || outAdj.empty())
        return 0;
    bool nonWeak = false;
    size_t outAdjSz = outAdj.size();
    for (size_t i = 0; i < outAdjSz; i++) {
        for (size_t j = 0; j < outAdj[i].size(); j++) {
            if (outLabel[i][j] != 0) {
                nonWeak = true;
                break;
            }
        }
        if (nonWeak)
            break;
    }
    if (!nonWeak)
        return 0;
    
    // Check clique
    // TODO: > 1 connected component, each a clique, is also counted as a clique here
    size_t numNodes = outLabel.size();
    if (numNodes == 1)
        return 1;   // 1 node is a clique with or without self-loop
    bool isClique = true;
    for (size_t i = 0; i < numNodes; i++) {
        unordered_set<int> neighbors;
        for (size_t j = 0; j < outAdj[i].size(); j++)
            neighbors.emplace(outAdj[i][j]);
        if (neighbors.size() != numNodes - 1) {
            isClique = false;
            break;
        }
    }
    if (isClique)
        return 1;

    return 2;
}

/**
 * @brief Constructs a line graph representation of the NFA for query processing
 *
 * This function builds a line graph from the NFA structure where each edge in the original
 * NFA becomes a node in the line graph. The line graph is used for efficient path query
 * execution and supports different types of connections:
 * - Label 1: source-to-source connections (bidirectional)
 * - Label 2: source-to-target connections
 * - Label 3: target-to-source connections
 * - Label 4: target-to-target connections (bidirectional)
 *
 * The algorithm performs a depth-first traversal of the NFA states and creates line graph
 * nodes for each transition, establishing the appropriate connectivity patterns based on
 * the transition endpoints and directions.
 *
 * @note The line graph structure is stored in the member variable `lg`
 * @see LineGraph structure for connection type definitions
 */
void NFA::fillLineGraph() {
    stack<shared_ptr<State>> st;
    st.emplace(initial);
    unordered_set<int> vis;
    vis.emplace(initial->id);
    while (!st.empty()) {
        shared_ptr<State> cur = st.top();
        st.pop();
        for (const auto &oe : cur->outEdges) {
            if (vis.find(oe.dst->id) == vis.end()) {
                vis.emplace(oe.dst->id);
                st.emplace(oe.dst);                
            }
            int u = lg.nodeLabel.size();
            lg.nodeLabel.emplace_back(oe.lbl, oe.forward);
            lg.outAdj.emplace_back();
            lg.outLabel.emplace_back();
            for (int v : cur->lineNodesFrom) {
                lg.addEdge(u, v, 1);
                lg.addEdge(v, u, 1);
            }
            for (int v : cur->lineNodesTo) {
                lg.addEdge(u, v, 2);
                lg.addEdge(v, u, 3);
            }
            for (int v : (oe.dst)->lineNodesFrom) {
                lg.addEdge(u, v, 3);
                lg.addEdge(v, u, 2);
            }
            for (int v : (oe.dst)->lineNodesTo) {
                lg.addEdge(u, v, 4);
                lg.addEdge(v, u, 4);
            }
            cur->lineNodesFrom.emplace_back(u);
            (oe.dst)->lineNodesTo.emplace_back(u);
        }
    }
}

// Make sure the NFA is a DFA before calling
std::shared_ptr<NFA> NFA::minimizeDfa() {
    size_t numGroup = 2, numStates = states.size();
    std::vector<size_t> state2part(states.size(), 0);
    if (numStates == accepts.size())
        numGroup = 1;
    else {
        for (const auto &s : accepts)
            state2part[s->id] = 1;
    }
    size_t prevNumGroup = numGroup;
    do {
        prevNumGroup = numGroup;
        for (size_t i = 0; i < numGroup; i++) {
            unordered_map<int, size_t> nextGroup;    // Represent label -> group
            bool addGroup = false;
            for (size_t j = 0; j < numStates; j++) {
                if (state2part[j] == i) {
                    if (nextGroup.empty()) {
                        for (const auto &oe : states[j]->outEdges) {
                            int lblForward = oe.forward ? oe.lbl : -oe.lbl;
                            nextGroup[lblForward] = state2part[(oe.dst)->id];
                        }
                    } else {
                        bool needAdd = false;
                        if (nextGroup.size() != states[j]->outEdges.size())
                            needAdd = true;
                        else {
                            for (const auto &oe : states[j]->outEdges) {
                                int lblForward = oe.forward ? oe.lbl : -oe.lbl;
                                auto it = nextGroup.find(lblForward);
                                if (it == nextGroup.end() || it->second != state2part[(oe.dst)->id]) {
                                    needAdd = true;
                                    break;
                                }
                            }
                        }
                        if (needAdd) {
                            // All split into the same group, further split later, no impact on correctness (but may impact performance)
                            if (addGroup)
                                state2part[j] = numGroup - 1;
                            else {
                                state2part[j] = numGroup++;
                                if (numGroup == numStates) {
                                    // *this is already a minimal DFA
                                    return make_shared<NFA>(*this);
                                }
                                addGroup = true;
                            }
                            continue;
                        }
                    }

                    // for (const auto &oe : states[j]->outEdges) {
                    //     int lblForward = oe.forward ? oe.lbl : -oe.lbl;
                    //     auto it = nextGroup.find(lblForward);
                    //     if (it == nextGroup.end())
                    //         nextGroup[lblForward] = state2part[(oe.dst)->id];
                    //     else if (it->second != state2part[(oe.dst)->id]) {
                    //         // All split into the same group, further split later, no impact on correctness (but may impact performance)
                    //         if (addGroup)
                    //             state2part[j] = numGroup - 1;
                    //         else {
                    //             state2part[j] = numGroup++;
                    //             addGroup = true;
                    //         }
                    //         // nextGroup[numGroup] = state2part[(oe.dst)->id];
                    //         break;
                    //     }
                    // }
                }
            }
        }
    } while (prevNumGroup < numGroup);

    // Add edges between groups
    shared_ptr<NFA> ret = make_shared<NFA>();
    ret->states.resize(numGroup);
    for (size_t i = 0; i < numGroup; i++)
        ret->states[i] = make_shared<State>(i, false);
    for (size_t i = 0; i < numStates; i++) {
        for (const auto &oe : states[i]->outEdges)
            ret->states[state2part[i]]->addTransition(oe.lbl, oe.forward, ret->states[state2part[(oe.dst)->id]]);
    }

    // Initial & accept states
    ret->initial = ret->states[state2part[initial->id]];
    ret->unsetAccept();
    for (const auto &s : accepts)
        ret->setAccept(ret->states[state2part[s->id]]);
    ret->curMaxId = numGroup;
    return ret;
}

void NFA::removeSelfLoop() {
    // For each self loop, create a new state to replace it
    size_t numStates = states.size();
    for (size_t i = 0; i < numStates; i++) {
        shared_ptr<State> addedState = nullptr;
        for (size_t j = 0; j < states[i]->outEdges.size(); j++) {
            if (states[i]->outEdges[j].dst == states[i]) {
                if (!addedState)
                    addedState = addState();
                addedState->addTransition(states[i]->outEdges[j].lbl, states[i]->outEdges[j].forward, states[i]);
                states[i]->outEdges.erase(states[i]->outEdges.begin() + j);
                j--;
            }
        }
        if (addedState) {
            for (const auto &oe : addedState->outEdges)
                states[i]->addTransition(oe.lbl, oe.forward, addedState);
            if (states[i]->accept)
                setAccept(addedState);
        }
    }
}