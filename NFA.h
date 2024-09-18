/**
 * @file NFA.h
 * @author Yue Pang 
 * @brief Nondeterministic finite automaton
 * @date 2022-08-24
 */

#pragma once

#include "Util.h"
#include "CSR.h"

struct State;    // Forward definition for Transition

struct Transition
{
    int lbl;    // -1 stands for eps
    bool forward;
    std::shared_ptr<State> dst;
    Transition();
    Transition(int lbl_, bool forward_, std::shared_ptr<State> dst_);
};

struct State
{
    int id;
    std::unordered_set<int> idSet;    // For DFA converted from NFA
    std::vector<Transition> outEdges;
    std::vector<int> lineNodesFrom, lineNodesTo;

    bool accept;
    State(int id_, bool accept_=false): id(id_), accept(accept_) {}
    void addTransition(int lbl_, bool forward_, std::shared_ptr<State> dst_);
    void addTransition(const Transition &tr);
    void print();
};

struct LineGraph {
    std::vector<std::pair<int, bool>> nodeLabel; // 0: weak connection, 1: s-s, 2: s-t, 3: t-s, 4: t-t. Second is whether forward
    std::vector<std::vector<int>> outAdj, outLabel;
    void addNode(const std::pair<int, bool> &pr) {
        nodeLabel.emplace_back(pr.first, pr.second);
        outAdj.emplace_back();
        outLabel.emplace_back();
    }
    void addEdge(int u, int v, int lbl) {
        outAdj[u].emplace_back(v);
        outLabel[u].emplace_back(lbl);
    }
    LineGraph operator * (const LineGraph &lg) const;
    LineGraph operator *= (const LineGraph &lg) {
        LineGraph prod = *this * lg;
        std::swap(prod, *this);
        return *this;
    }
    // Return edge label if found (the first edge found), -1 otherwise
    int findEdge(int u, int v) const {
        if (u >= int(outAdj.size())) return -1;
        for (size_t i = 0; i < outAdj[u].size(); i++)
            if (outAdj[u][i] == v) return outLabel[u][i];
        return -1;
    }
    int getType() const;
};

/**
 * @brief Represents both NFA and DFA (which is a special case of NFA)
 * 
 */
struct NFA
{
    std::vector<std::shared_ptr<State>> states;
    std::shared_ptr<State> initial;
    std::vector<std::shared_ptr<State>> accepts;    // Prevents searching for accept states on the fly
    int curMaxId;
    LineGraph lg;

    std::shared_ptr<State> addState(bool accept_);
    void addStates(std::vector<std::shared_ptr<State>> someStates);
    std::shared_ptr<State> id2state(int id_);
    std::shared_ptr<State> idSet2state(std::unordered_set<int> idSet_);
    void setAccept(std::shared_ptr<State> someState);
    void setAccept(std::vector<std::shared_ptr<State>> someStates);
    void unsetAccept(std::shared_ptr<State> someState);
    void unsetAccept();
    bool isAccept(std::shared_ptr<State> someState) const
    { return std::find(accepts.begin(), accepts.end(), someState) != accepts.end(); }
    void print();

    std::shared_ptr<NFA> convert2Dfa();
    void findEpsClosure(std::unordered_map<int, std::unordered_set<int>> &closures);
    void reverse();

    int **vis;
    bool outerVis;
    std::shared_ptr<MappedCSR> execute(std::shared_ptr<const MultiLabelCSR> csrPtr);
    bool checkIfValidSrc(size_t dataNode, std::shared_ptr<const MultiLabelCSR> csrPtr, int curVisMark);
    void clearVis(unsigned gN);

    NFA(): curMaxId(0), vis(nullptr), outerVis(false) {
        initial = addState(true);
        setAccept(initial);
    }
    ~NFA() {
        if (vis && !outerVis) {
            for (unsigned i = 0; i < states.size(); i++)
                delete []vis[i];
            delete []vis;
        }
    }
    void setVis(int **vis_) {
        vis = vis_;
        outerVis = true;
    }
    std::shared_ptr<NFA> minimizeDfa();
    void fillLineGraph();
    void removeSelfLoop();
};