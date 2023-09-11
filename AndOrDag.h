#pragma once
#include "CSR.h"

struct LabelOrInverse {
    double lbl;
    bool inv;
    LabelOrInverse(double lbl_, bool inv_): lbl(lbl_), inv(inv_) {}
};

class AndOrDagNode {
    bool isEq; // Equivalence or operation node
    char opType;    // Operation type: alternation 0, concat 1, * 2, + 3, ? 4
    std::vector<size_t> childIdx;   // Child node indices
    std::vector<size_t> parentIdx;  // Parent node indices
    std::vector<LabelOrInverse> startLabel, endLabel; // Start/end labels for equivalence nodes
    int topoOrder;    // Topological order of the node
    size_t targetChild; // Target child for equivalence nodes whose children are concat nodes
public:
    AndOrDagNode(): isEq(true), opType(0), topoOrder(-1), targetChild(0) {}
    AndOrDagNode(bool isEq_, char opType_): isEq(isEq_), opType(opType_), topoOrder(-1), targetChild(0) {}
    void addChild(size_t c) { childIdx.emplace_back(c); }
    void addParent(size_t c) { parentIdx.emplace_back(c); }
    void setIsEq(bool isEq_) { isEq = isEq_; }
    void setOpType(char opType_) { opType = opType_; }
    bool getIsEq() const { return isEq; }
    char getOpType() const { return opType; }
    const std::vector<size_t> &getChildIdx() const { return childIdx; }
    const std::vector<size_t> &getParentIdx() const { return parentIdx; }
    void addStartLabel(const std::vector<LabelOrInverse> &sl) { startLabel.insert(startLabel.end(), sl.begin(), sl.end()); }
    void addEndLabel(const std::vector<LabelOrInverse> &el) { endLabel.insert(endLabel.end(), el.begin(), el.end()); }
    void addStartLabel(double lbl_, bool inv_) { startLabel.emplace_back(lbl_, inv_); }
    void addEndLabel(double lbl_, bool inv_) { endLabel.emplace_back(lbl_, inv_); }
    const std::vector<LabelOrInverse> &getStartLabel() const { return startLabel; }
    const std::vector<LabelOrInverse> &getEndLabel() const { return endLabel; }
    void setTopoOrder(int topoOrder_) { topoOrder = topoOrder_; }
    int getTopoOrder() const { return topoOrder; }
    void setTargetChild(size_t targetChild_) { targetChild = targetChild_; }
};

class AndOrDag {
    std::vector<AndOrDagNode> nodes;
    std::unordered_map<std::string, size_t> q2idx;
    std::vector<bool> materialized;
    std::vector<float> cost;
    std::vector<size_t> workloadFreq;

    // Cardinality stuff
    std::vector<size_t> srcCnt, dstCnt;
    std::vector<size_t> useCnt; // useCnt assumes the current subquery is used whenever possible, UB on any actual case
    std::vector<float> pairProb;

    // View results
    std::vector<std::shared_ptr<MappedCSR>> res;

    // Pointer to statistics
    std::shared_ptr<const MultiLabelCSR> csrPtr;

public:
    AndOrDag(): csrPtr(nullptr) {}
    AndOrDag(std::shared_ptr<const MultiLabelCSR> csrPtr_): csrPtr(csrPtr_) {}
    void addWorkloadQuery(const std::string &q, size_t freq);   // Add the query q to the dag and mark as workload query
    int addQuery(const std::string &q);   // Add the query q to the dag
    void initAuxiliary();   // Call after finished constructing the dag
    void annotateLeafCostCard(); // Annotate leaf nodes' srcCnt, dstCnt, pairProb, cost
    float chooseMatViews(char mode, size_t spaceBudget=std::numeric_limits<size_t>::max(), std::string *testOut=nullptr);
    void plan();    // Plan the execution of the dag
    void propagateUseCnt(size_t idx); // Propagate useCnt from the current node
    void replanWithMaterialize(const std::vector<size_t> &matIdx, std::unordered_map<size_t, float> &node2cost, float &reducedCost); // Replan the dag assuming the input views are materialized
    void applyChanges(const std::unordered_map<size_t, float> &node2cost);    // Apply the changes from replan to the dag
    void updateNodeCost(size_t nodeIdx, std::unordered_map<size_t, float> &node2cost, float &reducedCost, float updateCost=-1); // Update the cost of a node (and its ancestors); -1 means update to cardinality
    void planNode(size_t nodeIdx);
    void execute(const std::string &q, std::shared_ptr<MappedCSR> resPtr); // Execute a query with the dag
    void executeNode(size_t nodeIdx, std::shared_ptr<MappedCSR> resPtr); // Execute a node with the dag
    void serialize();   // Serialize the dag to a file
    void deserialize(); // Deserialize the dag from a file

    void addNode(bool isEq_, char opType_) {
        nodes.emplace_back(isEq_, opType_);
        useCnt.emplace_back(0);
        workloadFreq.emplace_back(0);
    }
    void setAsWorkloadQuery(const std::string &q, size_t useCnt_, int workloadFreq_=-1) {
        // For testing only
        auto it = q2idx.find(q);
        if (it == q2idx.end())
            return;
        useCnt[it->second] = useCnt_;
        if (workloadFreq_ != -1)
            workloadFreq[it->second] = workloadFreq_;
        else
            workloadFreq[it->second] = useCnt_;
    }
    
    size_t getNumNodes() const { return nodes.size(); }
    const std::vector<AndOrDagNode> &getNodes() const { return nodes; }
    std::vector<AndOrDagNode> &getNodes() { return nodes; }
    const std::unordered_map<std::string, size_t> &getQ2idx() const { return q2idx; }
    std::unordered_map<std::string, size_t> &getQ2idx() { return q2idx; }
    const std::vector<size_t> &getSrcCnt() const { return srcCnt; }
    const std::vector<size_t> &getDstCnt() const { return dstCnt; }
    const std::vector<float> &getPairProb() const { return pairProb; }
    const std::vector<float> &getCost() const { return cost; }
    std::vector<size_t> &getWorkloadFreq() { return workloadFreq; }
    std::vector<size_t> &getUseCnt() { return useCnt; }
    void setSrcCnt(size_t idx, size_t srcCnt_) { srcCnt[idx] = srcCnt_; }
    void setDstCnt(size_t idx, size_t dstCnt_) { dstCnt[idx] = dstCnt_; }
    void setPairProb(size_t idx, float pairProb_) { pairProb[idx] = pairProb_; }
    void setCost(size_t idx, float cost_) { cost[idx] = cost_; }
    void setCsrPtr(std::shared_ptr<const MultiLabelCSR> &csrPtr_) { csrPtr = csrPtr_; }
    std::pair<float, float> getLeftRightWeight(const std::vector<LabelOrInverse> &lEnd, const std::vector<LabelOrInverse> &rStart);
    void addParentChild(size_t p, size_t c) {
        nodes[p].addChild(c);
        nodes[c].addParent(p);
    }
    void topoSort();
    bool isMaterialized(size_t idx) const { return materialized[idx]; }
};