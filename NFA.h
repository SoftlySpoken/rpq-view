/**
 * @file NFA.h
 * @author Yue Pang (michelle.py@pku.edu.cn)
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
    // TODO: state ID only need to be unique within an NFA/DFA
    int id;
    std::unordered_set<int> idSet;    // For DFA converted from NFA
    std::vector<Transition> outEdges;
    bool accept;
    State(int id_, bool accept_=false): id(id_), accept(accept_) {}
    void addTransition(int lbl_, bool forward_, std::shared_ptr<State> dst_);
    void addTransition(const Transition &tr);
    void print();
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
    std::shared_ptr<MappedCSR> execute(std::shared_ptr<const MultiLabelCSR> csrPtr);
    bool checkIfValidSrc(size_t dataNode, std::shared_ptr<const MultiLabelCSR> csrPtr);
    void clearVis(unsigned gN);

    NFA(): curMaxId(0), vis(nullptr) {
        initial = addState(true);
        setAccept(initial);
    }
    ~NFA() {
        if (vis) {
            for (unsigned i = 0; i < states.size(); i++)
                delete []vis[i];
            delete []vis;
        }
    }
};