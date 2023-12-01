#pragma once
#include "CSR.h"
#include "Rpq2NFAConvertor.h"
#define SAMPLESZ 100
#define NUMSTATES 20

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
    size_t targetChild; // Target child for equivalence nodes whose children are concat nodes (nodeIdx, not idx in childIdx)
    bool left2right;    // For concat op nodes, whether execute from left to right; for Kleene op nodes, whether fix-point (true) or no loop caching (false)
    std::shared_ptr<NFA> dfaPtr;    // DFA for equivalence nodes
    QueryResult res;  // Result pointer for materialized nodes
public:
    AndOrDagNode(): isEq(true), opType(0), topoOrder(-1), targetChild(0), left2right(true), dfaPtr(nullptr), res(nullptr, false) {}
    AndOrDagNode(bool isEq_, char opType_): isEq(isEq_), opType(opType_), topoOrder(-1), targetChild(0), left2right(true), dfaPtr(nullptr), res(nullptr, false) {}
    ~AndOrDagNode() { if (res.newed) delete res.csrPtr; }
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
    size_t getTargetChild() const { return targetChild; }
    void setLeft2Right(bool left2right_) { left2right = left2right_; }
    bool getLeft2Right() const { return left2right; }
    std::shared_ptr<NFA> getDfaPtr() const { return dfaPtr; }
    QueryResult &getRes() { return res; }
    const QueryResult &getRes() const { return res; }
};

class AndOrDag {
    std::vector<AndOrDagNode> nodes;
    std::unordered_map<std::string, size_t> q2idx;
    std::vector<std::string> idx2q;
    std::vector<bool> materialized;
    std::vector<float> cost;
    std::vector<size_t> workloadFreq;

    // Cardinality stuff
    std::vector<size_t> srcCnt, dstCnt;
    std::vector<size_t> card;
    std::vector<size_t> freq; // UB on any actual useCnt
    std::vector<int> useCnt;    // actual useCnt, int for easier subtraction

    std::shared_ptr<MultiLabelCSR> csrPtr;

    int **vis;
    int curVisMark;

public:
    AndOrDag(): csrPtr(nullptr), vis(nullptr), curVisMark(INT_MIN) {}
    AndOrDag(std::shared_ptr<MultiLabelCSR> csrPtr_): csrPtr(csrPtr_), vis(nullptr), curVisMark(INT_MIN) { clearVis(); }
    AndOrDag(const AndOrDag &aod_): nodes(aod_.nodes), q2idx(aod_.q2idx), idx2q(aod_.idx2q), materialized(aod_.materialized),
    cost(aod_.cost), workloadFreq(aod_.workloadFreq), srcCnt(aod_.srcCnt), dstCnt(aod_.dstCnt), card(aod_.card), freq(aod_.freq),
    useCnt(aod_.useCnt), csrPtr(aod_.csrPtr), vis(nullptr), curVisMark(INT_MIN) {
        // Copy constructor avoid vis double delete
        clearVis();
    }
    ~AndOrDag() {
        if (vis) {
            for (size_t j = 0; j < NUMSTATES; j++)
                delete []vis[j];
            delete []vis;
        }
    }
    void clearVis(int idx=-1) {
        size_t gN = csrPtr->maxNode + 1;
        if (!vis) {
            vis = new int *[NUMSTATES];
            for (size_t j = 0; j < NUMSTATES; j++) {
                vis[j] = new int [gN];
                if (idx == -1 || j == size_t(idx))
                    memset(vis[j], -1, gN * sizeof(int));
            }
        } else {
            if (idx == -1) {
                for (size_t j = 0; j < NUMSTATES; j++)
                    memset(vis[j], -1, gN * sizeof(int));
            } else
                memset(vis[idx], -1, gN * sizeof(int));
        }
    }
    void addWorkloadQuery(const std::string &q, size_t curFreq);   // Add the query q to the dag and mark as workload query
    int addQuery(const std::string &q);   // Add the query q to the dag
    void initAuxiliary();   // Call after finished constructing the dag
    void annotateLeafCostCard(); // Annotate leaf nodes' srcCnt, dstCnt, pairProb, cost
    float chooseMatViews(char mode, size_t &usedSpace, size_t spaceBudget=std::numeric_limits<size_t>::max(), std::string *testOut=nullptr);
    void plan();    // Plan the execution of the dag
    void propagate();
    void propagateFreq(size_t idx, size_t propVal); // Propagate freq from the current node
    void propagateUseCnt(size_t idx, int delta);    // Propagate useCnt change from the current node
    void replanWithMaterialize(const std::vector<size_t> &matIdx, std::unordered_map<size_t, float> &node2cost, float &reducedCost); // Replan the dag assuming the input views are materialized
    void applyChanges(const std::vector<size_t> &matIdx, const std::unordered_map<size_t, float> &node2cost, bool updateUseCnt=false);    // Apply the changes from replan to the dag
    void updateNodeCost(size_t nodeIdx, std::unordered_map<size_t, float> &node2cost, float &reducedCost, float updateCost=-1); // Update the cost of a node (and its ancestors); -1 means update to cardinality
    void planNode(size_t nodeIdx);
    void materialize(); // Materialize the chosen views
    void execute(const std::string &q, QueryResult &qr); // Execute a query with the dag
    // Execute a node with the dag
    void executeNode(size_t nodeIdx, QueryResult &qr, const std::unordered_set<size_t> *lCandPtr=nullptr,
        const std::unordered_set<size_t> *rCandPtr=nullptr, QueryResult *nlcResPtr=nullptr, int curMatIdx=-1);
    void serialize();   // Serialize the dag to a file
    void deserialize(); // Deserialize the dag from a file

    void addNode(bool isEq_, char opType_) {
        nodes.emplace_back(isEq_, opType_);
        freq.emplace_back(0);
        useCnt.emplace_back(0);
        workloadFreq.emplace_back(0);
    }
    void setAsWorkloadQuery(const std::string &q, size_t useCnt_, int workloadFreq_=-1) {
        // For testing only
        auto it = q2idx.find(q);
        if (it == q2idx.end())
            return;
        freq[it->second] = useCnt_;
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
    const std::vector<size_t> &getCard() const { return card; }
    const std::vector<float> &getCost() const { return cost; }
    std::vector<size_t> &getWorkloadFreq() { return workloadFreq; }
    std::vector<size_t> &getFreq() { return freq; }
    std::vector<int> &getUseCnt() { return useCnt; }
    std::vector<bool> &getMaterialized() { return materialized; }
    void setSrcCnt(size_t idx, size_t srcCnt_) { srcCnt[idx] = srcCnt_; }
    void setDstCnt(size_t idx, size_t dstCnt_) { dstCnt[idx] = dstCnt_; }
    void setCost(size_t idx, float cost_) { cost[idx] = cost_; }
    void setCard(size_t idx, size_t card_) { card[idx] = card_; }
    void setCsrPtr(std::shared_ptr<MultiLabelCSR> &csrPtr_) { csrPtr = csrPtr_; clearVis(); }
    void addParentChild(size_t p, size_t c) {
        nodes[p].addChild(c);
        nodes[c].addParent(p);
    }
    void topoSort();
    bool isMaterialized(size_t idx) const {
        if (idx >= nodes.size())
            return false;
        return materialized[idx];
    }
    void setMaterialized(size_t idx) {
        if (idx < nodes.size())
            materialized[idx] = true;
    }
    float approxMiddleDivInMonteCarlo(const std::vector<LabelOrInverse> &endLabelVec, size_t nodeIdx);
};