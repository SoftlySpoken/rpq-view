#include <gtest/gtest.h>
#include "AndOrDag.h"
using namespace std;

// AND-OR DAG file format: (expected output of CustomTest, input of buildAndOrDagFromFile)
// nodes
// #nodes
// each node: isEq opType #children child1 child2 … #startLabel lbl1 inv1 lbl2 inv2 ... #endLabel lbl1 inv1 lbl2 inv2 ...
// q2idx
// #q
// each line: q idx
// (Below is optional content)
// #eq nodes that need to specify targetChild
// each line: idx targetChild
void CustomTest(const std::string& testName) {
    // Generate input and expected output file names based on the test name
    string dataDir = "../test_data/AddQueryTestSuite/";
    std::string inputFileName = dataDir + testName + "_query.txt";
    std::string expectedOutputFileName = dataDir + testName + "_expected_output.txt";
    
    AndOrDag aod;

    // Read input data from the input file
    std::ifstream inputFile(inputFileName);
    cout << inputFileName << endl;
    ASSERT_EQ(inputFile.is_open(), true);
    string cur;
    vector<string> qVec;
    while (inputFile >> cur)
        qVec.emplace_back(cur);
    inputFile.close();

    // Calculate the result using the function being tested
    for (const auto &q : qVec)
        aod.addWorkloadQuery(q, 1);

    // Read the expected output from the expected output file
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);

    // Compare the actual result with the expected result
    size_t numNodes = 0;
    expectedOutputFile >> numNodes;
    ASSERT_EQ(aod.getNumNodes(), numNodes);
    bool isEq = false;
    int opType = 0;
    size_t numChildren = 0, curChild = 0, numLabel = 0, curLbl = 0;
    bool isInv = false;
    for (size_t i = 0; i < numNodes; i++) {
        expectedOutputFile >> isEq >> opType >> numChildren;
        EXPECT_EQ(aod.getNodes()[i].getIsEq(), isEq);
        EXPECT_EQ(int(aod.getNodes()[i].getOpType()), opType);
        EXPECT_EQ(aod.getNodes()[i].getChildIdx().size(), numChildren);
        for (size_t j = 0; j < numChildren; j++) {
            expectedOutputFile >> curChild;
            EXPECT_EQ(aod.getNodes()[i].getChildIdx()[j], curChild);
        }
        expectedOutputFile >> numLabel;
        for (size_t j = 0; j < numLabel; j++) {
            expectedOutputFile >> curLbl >> isInv;
            EXPECT_EQ(aod.getNodes()[i].getStartLabel()[j].lbl, curLbl);
            EXPECT_EQ(aod.getNodes()[i].getStartLabel()[j].inv, isInv);
        }
        expectedOutputFile >> numLabel;
        for (size_t j = 0; j < numLabel; j++) {
            expectedOutputFile >> curLbl >> isInv;
            EXPECT_EQ(aod.getNodes()[i].getEndLabel()[j].lbl, curLbl);
            EXPECT_EQ(aod.getNodes()[i].getEndLabel()[j].inv, isInv);
        }
    }

    size_t numQ = 0;
    expectedOutputFile >> numQ;
    ASSERT_EQ(aod.getQ2idx().size(), numQ);
    size_t qIdx = 0;
    for (size_t i = 0; i < numQ; i++) {
        expectedOutputFile >> cur;
        const auto &it = aod.getQ2idx().find(cur);
        EXPECT_EQ(it != aod.getQ2idx().end(), true);
        expectedOutputFile >> qIdx;
        EXPECT_EQ(it->second, qIdx);
    }
}

void buildAndOrDagFromFile(AndOrDag &aod, const string &inputFileName, bool getTargetChild=false, bool l2r=true) {
    std::ifstream inputFile(inputFileName);
    ASSERT_EQ(inputFile.is_open(), true);
    size_t numNodes = 0;
    inputFile >> numNodes;
    bool isEq = false;
    int opType = 0;
    size_t numChildren = 0, curChild = 0, numLabel = 0, curLabel = 0;
    bool isInv = false;
    aod.getNodes().resize(numNodes);
    aod.getWorkloadFreq().resize(numNodes);
    for (size_t i = 0; i < numNodes; i++) {
        inputFile >> isEq >> opType >> numChildren;
        aod.getNodes()[i].setIsEq(isEq);
        aod.getNodes()[i].setOpType(opType);
        aod.getNodes()[i].setLeft2Right(l2r);
        for (size_t j = 0; j < numChildren; j++) {
            inputFile >> curChild;
            aod.addParentChild(i, curChild);
        }
        inputFile >> numLabel;
        for (size_t j = 0; j < numLabel; j++) {
            inputFile >> curLabel >> isInv;
            aod.getNodes()[i].addStartLabel(curLabel, isInv);
        }
        inputFile >> numLabel;
        for (size_t j = 0; j < numLabel; j++) {
            inputFile >> curLabel >> isInv;
            aod.getNodes()[i].addEndLabel(curLabel, isInv);
        }
    }
    size_t numQ = 0;
    inputFile >> numQ;
    size_t qIdx = 0;
    string cur;
    for (size_t i = 0; i < numQ; i++) {
        inputFile >> cur >> qIdx;
        aod.getQ2idx()[cur] = qIdx;
    }
    // Optionally set targetChild
    if (getTargetChild) {
        size_t numEq = 0;
        inputFile >> numEq;
        size_t curNodeIdx = 0, curTargetChild = 0;
        for (size_t i = 0; i < numEq; i++) {
            inputFile >> curNodeIdx >> curTargetChild;
            aod.getNodes()[curNodeIdx].setTargetChild(curTargetChild);
        }
    }
    inputFile.close();
}

class ExecuteTestSuite : public ::testing::TestWithParam<std::pair<std::string, bool>> {
protected:
    std::shared_ptr<MultiLabelCSR> csrPtr;
    std::string dataDir;
    ExecuteTestSuite(): csrPtr(nullptr), dataDir("../test_data/ExecuteTestSuite/") {}
    void SetUp() override {
        string graphFilePath = dataDir + "graph.txt";
        csrPtr = make_shared<MultiLabelCSR>();
        csrPtr->loadGraph(graphFilePath);
    }
};

class ChooseMatViewsTestSuite : public ::testing::TestWithParam<vector<size_t>> {
protected:
    AndOrDag aod;
    std::shared_ptr<MultiLabelCSR> csrPtr;
    ChooseMatViewsTestSuite(): csrPtr(nullptr) {}
    void SetUp() override {
        string dataDir = "../test_data/ChooseMatViewsTestSuite/";
        string graphFilePath = dataDir + "KleeneIriConcatTest_graph.txt";
        csrPtr = make_shared<MultiLabelCSR>();
        csrPtr->loadGraph(graphFilePath);

        aod.setCsrPtr(csrPtr);
        string inputFileName = dataDir + "KleeneIriConcatTest_input.txt";
        buildAndOrDagFromFile(aod, inputFileName);
        aod.initAuxiliary();
        aod.getFreq().assign(aod.getNumNodes(), 0);
        aod.getUseCnt().assign(aod.getNumNodes(), 0);
        string costFileName = dataDir + "KleeneIriConcatTest_cost.txt";
        std::ifstream costFile(costFileName);
        ASSERT_EQ(costFile.is_open(), true);
        size_t numLines = 0;
        costFile >> numLines;
        size_t curSrcCnt = 0, curDstCnt = 0, curCard = 0;
        float curCost = 0;
        for (size_t i = 0; i < numLines; i++) {
            costFile >> curCost >> curSrcCnt >> curDstCnt >> curCard;
            aod.setCost(i, curCost);
            aod.setSrcCnt(i, curSrcCnt);
            aod.setDstCnt(i, curDstCnt);
            aod.setCard(i, curCard);
        }
        costFile.close();
        string queryFileName = dataDir + "KleeneIriConcatTest_query2freq.txt";
        std::ifstream queryFile(queryFileName);
        ASSERT_EQ(queryFile.is_open(), true);
        string q;
        size_t useCnt_ = 0, workloadFreq_ = 0;
        while (queryFile >> q >> useCnt_ >> workloadFreq_)
            aod.setAsWorkloadQuery(q, useCnt_, workloadFreq_);
        queryFile.close();
    }
};

// Define test cases using the custom test case function
TEST(AddQueryTestSuite, SingleIriTest) {
    CustomTest("SingleIriTest");
}

TEST(AddQueryTestSuite, SingleInverseIriTest) {
    CustomTest("SingleInverseIriTest");
}

TEST(AddQueryTestSuite, SingleParenthesizedIriTest) {
    CustomTest("SingleParenthesizedIriTest");
}

TEST(AddQueryTestSuite, ConcatTest) {
    CustomTest("ConcatTest");
}

TEST(AddQueryTestSuite, AlternationTest) {
    CustomTest("AlternationTest");
}

TEST(AddQueryTestSuite, SingleIriKleeneTest) {
    CustomTest("SingleIriKleeneTest");
}

TEST(AddQueryTestSuite, ConcatKleeneTest) {
    CustomTest("ConcatKleeneTest");
}

TEST(AddQueryTestSuite, KleeneIriConcatTest) {
    CustomTest("KleeneIriConcatTest");
}

TEST(AddQueryTestSuite, QuerySubqueryTest) {
    CustomTest("QuerySubqueryTest");
}

TEST(AddQueryTestSuite, OverlapTest) {
    CustomTest("OverlapTest");
}

TEST(UseCntTestSuite, TwoRootsTest) {
    string dataDir = "../test_data/UseCntTestSuite/", testName = "TwoRootsTest";
    std::string inputFileName = dataDir + testName + "_input.txt";
    std::string expectedOutputFileName = dataDir + testName + "_expected_output.txt";
    
    AndOrDag aod;
    string q;
    size_t freq;
    std::ifstream inputFile(inputFileName);
    ASSERT_EQ(inputFile.is_open(), true);
    while (inputFile >> q >> freq)
        aod.addWorkloadQuery(q, freq);
    inputFile.close();
    aod.getNodes()[0].setTargetChild(1);
    aod.getNodes()[10].setTargetChild(11);
    aod.getMaterialized().assign(aod.getNodes().size(), false);
    // Simulate the propagation of freq
    aod.propagate();

    vector<size_t> realFreq;
    ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    while (expectedOutputFile >> freq)
        realFreq.emplace_back(freq);
    ASSERT_EQ(aod.getFreq().size(), realFreq.size());
    for (size_t i = 0; i < realFreq.size(); i++)
        EXPECT_EQ(aod.getFreq()[i], realFreq[i]);
}

void useCntTest(const std::string& testName) {
    string dataDir = "../test_data/UseCntTestSuite/";
    std::string inputFileName = dataDir + testName + "_input.txt";
    std::string expectedOutputFileName = dataDir + testName + "_expected_output.txt";
    
    AndOrDag aod;
    string q;
    size_t freq;
    std::ifstream inputFile(inputFileName);
    ASSERT_EQ(inputFile.is_open(), true);
    while (inputFile >> q >> freq)
        aod.addWorkloadQuery(q, freq);
    inputFile.close();
    aod.getNodes()[0].setTargetChild(1);
    aod.getMaterialized().assign(aod.getNodes().size(), false);
    // Simulate the propagation of freq
    aod.propagate();
    vector<vector<int>> useCntVec;
    useCntVec.emplace_back(aod.getUseCnt());

    aod.propagateUseCnt(1, -1);
    aod.getNodes()[0].setTargetChild(7);
    aod.propagateUseCnt(7, 1);
    size_t tmpUseCnt = aod.getUseCnt()[8];
    aod.propagateUseCnt(8, 0 - tmpUseCnt);
    aod.getUseCnt()[8] = tmpUseCnt;
    aod.getMaterialized()[8] = true;
    useCntVec.emplace_back(aod.getUseCnt());

    tmpUseCnt = aod.getUseCnt()[0];
    aod.propagateUseCnt(0, 0 - tmpUseCnt);
    aod.getUseCnt()[0] = tmpUseCnt;
    aod.getMaterialized()[0] = true;
    useCntVec.emplace_back(aod.getUseCnt());

    ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    size_t numElems = 0;
    expectedOutputFile >> numElems;
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQ(useCntVec[i].size(), numElems);
        for (size_t j = 0; j < numElems; j++) {
            expectedOutputFile >> tmpUseCnt;
            EXPECT_EQ(useCntVec[i][j], tmpUseCnt);
        }
    }
}

TEST(UseCntTestSuite, SingleRootInclusionTest) {
    useCntTest("SingleRootInclusionTest");
}

TEST(UseCntTestSuite, SingleRootTest) {
    useCntTest("SingleRootTest");
}

TEST(AnnotateLeafCostCardTestSuite, SimpleTest) {
    // Construct outCsr, inCsr for 0-[0]->1-[1]->2-[2]->3-[3]->4
    auto csrPtr = make_shared<MultiLabelCSR>();
    csrPtr->outCsr.resize(4);
    csrPtr->inCsr.resize(4);
    for (size_t i = 0; i < 4; i++) {
        csrPtr->label2idx[i] = i;
        csrPtr->outCsr[i].n = 1;
        csrPtr->outCsr[i].m = 1;
        csrPtr->outCsr[i].adj.emplace_back(i + 1);
        csrPtr->outCsr[i].offset = {0};
        csrPtr->outCsr[i].v2idx[i] = 0;
        csrPtr->inCsr[i].n = 1;
        csrPtr->inCsr[i].m = 1;
        csrPtr->inCsr[i].adj.emplace_back(i);
        csrPtr->inCsr[i].offset = {0};
        csrPtr->inCsr[i].v2idx[i + 1] = 0;
    }

    // Construct AndOrDag for (<1>/<2>)+/<3>
    AndOrDag aod(csrPtr);
    string dataDir = "../test_data/AnnotateLeafCostCardTestSuite/";
    string inputFileName = dataDir + "SimpleTest_input.txt";
    buildAndOrDagFromFile(aod, inputFileName);

    // Call annotateLeafCostCard
    aod.initAuxiliary();
    aod.annotateLeafCostCard();

    // Compare the actual result with the expected result
    // Output file format: #lines
    // each line nodeIdx srcCnt dstCnt pairProb cost (only leaf nodes)
    string expectedOutputFileName = dataDir + "SimpleTest_expected_output.txt";
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    size_t numLeafNodes = 0, nodeIdx = 0;
    expectedOutputFile >> numLeafNodes;
    size_t curSrcCnt = 0, curDstCnt = 0, curCard = 0;
    float curCost = 0;
    for (size_t i = 0; i < numLeafNodes; i++) {
        expectedOutputFile >> nodeIdx >> curSrcCnt >> curDstCnt >> curCard >> curCost;
        ASSERT_EQ(aod.getNumNodes() > nodeIdx, true);
        EXPECT_EQ(aod.getSrcCnt()[nodeIdx], curSrcCnt);
        EXPECT_EQ(aod.getDstCnt()[nodeIdx], curDstCnt);
        EXPECT_EQ(aod.getCard()[nodeIdx], curCard);
        EXPECT_FLOAT_EQ(aod.getCost()[nodeIdx], curCost);
    }
}

TEST(PlanTestSuite, KleeneIriConcatTest) {
    // Assume CSR loadGraph is correct
    string dataDir = "../test_data/PlanTestSuite/";
    string graphFilePath = dataDir + "KleeneIriConcatTest_graph.txt";
    auto csrPtr = make_shared<MultiLabelCSR>();
    csrPtr->loadGraph(graphFilePath);

    AndOrDag aod(csrPtr);
    string inputFileName = dataDir + "KleeneIriConcatTest_input.txt";
    buildAndOrDagFromFile(aod, inputFileName);
    aod.initAuxiliary();
    aod.getFreq().assign(aod.getNumNodes(), 0);
    aod.getUseCnt().assign(aod.getNumNodes(), 0);
    aod.annotateLeafCostCard();
    string queryFileName = dataDir + "KleeneIriConcatTest_query.txt";
    std::ifstream queryFile(queryFileName);
    ASSERT_EQ(queryFile.is_open(), true);
    string q;
    while (queryFile >> q)
        aod.setAsWorkloadQuery(q, 1);
    aod.plan();

    // Compare the actual result with the expected result
    string expectedOutputFileName = dataDir + "KleeneIriConcatTest_expected_output.txt";
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    size_t numLines = 0;
    expectedOutputFile >> numLines;
    const auto &cost = aod.getCost();
    const auto &srcCnt = aod.getSrcCnt(), &dstCnt = aod.getDstCnt();
    const auto &card = aod.getCard();
    size_t curSrcCnt = 0, curDstCnt = 0, curCard = 0;
    float curCost = 0;
    for (size_t i = 0; i < numLines; i++) {
        expectedOutputFile >> curCost >> curSrcCnt >> curDstCnt >> curCard;
        EXPECT_FLOAT_EQ(cost[i], curCost);
        EXPECT_EQ(srcCnt[i], curSrcCnt);
        EXPECT_EQ(dstCnt[i], curDstCnt);
        EXPECT_EQ(card[i], curCard);
    }
}

TEST(TopoSortTestSuite, KleeneIriConcatTest) {
    string dataDir = "../test_data/TopoSortTestSuite/";
    AndOrDag aod;
    string inputFileName = dataDir + "KleeneIriConcatTest_input.txt";
    buildAndOrDagFromFile(aod, inputFileName);
    aod.topoSort();
    string expectedOutputFileName = dataDir + "KleeneIriConcatTest_expected_output.txt";
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    size_t numNodes = 0, curOrder = 0;
    expectedOutputFile >> numNodes;
    for (size_t i = 0; i < numNodes; i++) {
        expectedOutputFile >> curOrder;
        EXPECT_EQ(aod.getNodes()[i].getTopoOrder(), curOrder);
    }
}

TEST(ReplanWithMaterializeTestSuite, KleeneIriConcatTest) {
    string dataDir = "../test_data/ReplanWithMaterializeTestSuite/";
    string graphFilePath = dataDir + "KleeneIriConcatTest_graph.txt";
    auto csrPtr = make_shared<MultiLabelCSR>();
    csrPtr->loadGraph(graphFilePath);

    AndOrDag aod(csrPtr);
    string inputFileName = dataDir + "KleeneIriConcatTest_input.txt";
    buildAndOrDagFromFile(aod, inputFileName);
    aod.initAuxiliary();
    aod.getFreq().assign(aod.getNumNodes(), 0);
    aod.getUseCnt().assign(aod.getNumNodes(), 0);
    string costFileName = dataDir + "KleeneIriConcatTest_cost.txt";
    std::ifstream costFile(costFileName);
    ASSERT_EQ(costFile.is_open(), true);
    size_t numLines = 0;
    costFile >> numLines;
    size_t curSrcCnt = 0, curDstCnt = 0, curCard = 0;
    float curCost = 0;
    for (size_t i = 0; i < numLines; i++) {
        costFile >> curCost >> curSrcCnt >> curDstCnt >> curCard;
        aod.setCost(i, curCost);
        aod.setSrcCnt(i, curSrcCnt);
        aod.setDstCnt(i, curDstCnt);
        aod.setCard(i, curCard);
    }
    costFile.close();
    string queryFileName = dataDir + "KleeneIriConcatTest_query.txt";
    std::ifstream queryFile(queryFileName);
    ASSERT_EQ(queryFile.is_open(), true);
    string q;
    while (queryFile >> q)
        aod.setAsWorkloadQuery(q, 1);
    queryFile.close();

    string matFileName = dataDir + "KleeneIriConcatTest_mat.txt";
    std::ifstream matFile(matFileName);
    ASSERT_EQ(matFile.is_open(), true);
    std::vector<size_t> matIdx;
    std::unordered_map<size_t, float> node2cost;
    float reducedCost = 0;
    while (matFile >> q) {
        auto it = aod.getQ2idx().find(q);
        ASSERT_EQ(it != aod.getQ2idx().end(), true);
        matIdx.emplace_back(it->second);
    }
    vector<size_t> vIdxAdded, vIdxRemoved;
    aod.replanWithMaterialize(matIdx, node2cost, reducedCost);

    // Compare the actual result with the expected result
    // File format: len(node2cost)
    // each line: nodeIdx cost
    // last line: reducedCost
    string expectedOutputFileName = dataDir + "KleeneIriConcatTest_expected_output.txt";
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    size_t numNode2cost = 0, curNodeIdx = 0;
    expectedOutputFile >> numNode2cost;
    ASSERT_EQ(node2cost.size(), numNode2cost);
    for (size_t i = 0; i < numNode2cost; i++) {
        expectedOutputFile >> curNodeIdx >> curCost;
        auto it = node2cost.find(curNodeIdx);
        ASSERT_EQ(it != node2cost.end(), true);
        EXPECT_FLOAT_EQ(it->second, curCost);
    }
    expectedOutputFile >> curCost;
    EXPECT_FLOAT_EQ(reducedCost, curCost);
    expectedOutputFile.close();
}

TEST_P(ChooseMatViewsTestSuite, KleeneIriConcatTest) {
    string testOutput;
    const auto &curParam = GetParam();
    size_t isCopy = curParam[0], curMode = curParam[1], curBudget = curParam[2];
    float retRealBenefit = 0;
    size_t usedSpace = 0;
    if (isCopy == 0)
        retRealBenefit = aod.chooseMatViews(curMode, usedSpace, curBudget, &testOutput);
    else {
        AndOrDag tmpAod(aod);
        retRealBenefit = tmpAod.chooseMatViews(curMode, usedSpace, curBudget, &testOutput);
    }

    // Compare the actual result with the expected result
    // File format: triples of (nodeIdx, satCond (1/0), realBenefit)
    // satCond: if true, enters the if branch; otherwise, enters the else branch
    string dataDir = "../test_data/ChooseMatViewsTestSuite/";
    string expectedOutputFileName = dataDir + "KleeneIriConcatTest_expected_output";
    expectedOutputFileName += "_" + std::to_string(curMode);
    if (curBudget == std::numeric_limits<size_t>::max())
        expectedOutputFileName += "_max";
    else
        expectedOutputFileName += "_" + std::to_string(curBudget);
    expectedOutputFileName += ".txt";
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    float totalRealBenefit = 0;
    if (curMode == 0) {
        vector<size_t> nodeIdxVec;
        size_t curNodeIdx = 0;
        vector<bool> satCondVec;
        bool curSatCond = 0;
        vector<float> realBenefitVec;
        float curRealBenefit = 0;
        stringstream ss(testOutput);
        while (ss >> curNodeIdx >> curSatCond >> curRealBenefit) {
            nodeIdxVec.emplace_back(curNodeIdx);
            satCondVec.emplace_back(curSatCond);
            realBenefitVec.emplace_back(curRealBenefit);
        }
        size_t i = 0;
        expectedOutputFile >> totalRealBenefit;
        EXPECT_FLOAT_EQ(retRealBenefit, totalRealBenefit);
        while (expectedOutputFile >> curNodeIdx >> curSatCond >> curRealBenefit) {
            EXPECT_EQ(nodeIdxVec[i], curNodeIdx);
            EXPECT_EQ(satCondVec[i], curSatCond);
            EXPECT_FLOAT_EQ(realBenefitVec[i], curRealBenefit);
            i++;
        }
    } else {
        vector<size_t> nodeIdxVec;
        vector<bool> decisionVec;
        size_t curNodeIdx = 0;
        bool curDecision = false;
        stringstream ss(testOutput);
        while (ss >> curNodeIdx >> curDecision) {
            nodeIdxVec.emplace_back(curNodeIdx);
            decisionVec.emplace_back(curDecision);
        }
        size_t i = 0;
        expectedOutputFile >> totalRealBenefit;
        EXPECT_FLOAT_EQ(retRealBenefit, totalRealBenefit);
        while (expectedOutputFile >> curNodeIdx >> curDecision) {
            EXPECT_EQ(nodeIdxVec[i], curNodeIdx);
            EXPECT_EQ(decisionVec[i], curDecision);
            i++;
        }
    }
}

// TODO: write a function to generate the params
INSTANTIATE_TEST_SUITE_P(ChooseMatViewsTestSuiteInstance, ChooseMatViewsTestSuite,
::testing::Values(vector<size_t>({0, 0, numeric_limits<size_t>::max()}), vector<size_t>({0, 0, 4}),
vector<size_t>({0, 1, numeric_limits<size_t>::max()}), vector<size_t>({0, 1, 4}),
vector<size_t>({0, 2, numeric_limits<size_t>::max()}), vector<size_t>({0, 2, 4}),
vector<size_t>({0, 3, numeric_limits<size_t>::max()}), vector<size_t>({0, 3, 4}),
vector<size_t>({0, 4, numeric_limits<size_t>::max()}), vector<size_t>({0, 4, 4}),
vector<size_t>({1, 0, numeric_limits<size_t>::max()}), vector<size_t>({1, 0, 4}),
vector<size_t>({1, 1, numeric_limits<size_t>::max()}), vector<size_t>({1, 1, 4}),
vector<size_t>({1, 2, numeric_limits<size_t>::max()}), vector<size_t>({1, 2, 4}),
vector<size_t>({1, 3, numeric_limits<size_t>::max()}), vector<size_t>({1, 3, 4}),
vector<size_t>({1, 4, numeric_limits<size_t>::max()}), vector<size_t>({1, 4, 4})
));

void compareExecuteResult(const string &expectedOutputFileName, MultiLabelCSR *dataCsrPtr,
MappedCSR *resCsrPtr, bool explicitEpsilon=false) {
    // Expected query result output file format: #nodes
    // each line: nodeIdx #neighbors neighbor1 neighbor2 ...
    std::ifstream expectedOutputFile(expectedOutputFileName);
    ASSERT_EQ(expectedOutputFile.is_open(), true);
    size_t numNodes = 0;
    expectedOutputFile >> numNodes;
    size_t curNodeIdx = 0, numNeighbors = 0, curNeighbor = 0;
    unordered_map<size_t, unordered_set<size_t>> realAdjList;
    AdjInterval aitv;
    for (size_t i = 0; i < numNodes; i++) {
        expectedOutputFile >> curNodeIdx >> numNeighbors;
        realAdjList[curNodeIdx] = unordered_set<size_t>();
        for (size_t j = 0; j < numNeighbors; j++) {
            expectedOutputFile >> curNeighbor;
            realAdjList[curNodeIdx].emplace(curNeighbor);
        }
    }
    bool realHasEpsilon = false;
    expectedOutputFile >> realHasEpsilon;
    expectedOutputFile.close();
    if (explicitEpsilon && realHasEpsilon) {
        // For DFA execution (explicitEpsilon == true), explicitly enhance the expected output if hasEpsilon == true
        for (size_t i = 0; i <= dataCsrPtr->maxNode; i++) {
            if (realAdjList.find(i) == realAdjList.end())
                realAdjList[i] = unordered_set<size_t>();
            realAdjList[i].emplace(i);
        }
    }
    // Compare the query result with the ground truth
    // Do not compare the number of nodes, since node idx may not be continuous
    for (const auto &pr : resCsrPtr->v2idx) {
        size_t curNodeIdx = pr.first;
        ASSERT_EQ(realAdjList.find(curNodeIdx) != realAdjList.end(), true);
        resCsrPtr->getAdjIntervalByVert(curNodeIdx, aitv);
        ASSERT_EQ(aitv.len, realAdjList[curNodeIdx].size());
        for (size_t j = 0; j < aitv.len; j++)
            EXPECT_EQ(realAdjList[curNodeIdx].find((*aitv.start)[aitv.offset + j]) != realAdjList[curNodeIdx].end(), true);
    }
}

TEST_P(ExecuteTestSuite, ExecuteTest) {
    const auto &pr = GetParam();
    const string &testName = pr.first;
    bool mat = pr.second;
    // If materialize flag == true, read nodes to materialize from file; materialize the nodes
    vector<size_t> matIdx;
    if (mat) {
        string matIdxFileName = dataDir + testName + "_matIdx.txt";
        std::ifstream matIdxFile(matIdxFileName);
        ASSERT_EQ(matIdxFile.is_open(), true);
        size_t curMatIdx = 0;
        while (matIdxFile >> curMatIdx)
            matIdx.emplace_back(curMatIdx);
        matIdxFile.close();
    }
    AndOrDag aod;
    aod.setCsrPtr(csrPtr);
    string inputFileName = dataDir + testName + "_input.txt";
    if (testName == "ConcatTest")
        buildAndOrDagFromFile(aod, inputFileName, true, true);
    else
        buildAndOrDagFromFile(aod, inputFileName, false, true);
    aod.initAuxiliary();
    if (mat) {
        for (size_t i : matIdx)
            aod.setMaterialized(i);
        aod.materialize();
    }

    QueryResult qr(nullptr, false);
    string queryFileName = dataDir + testName + "_query.txt";
    std::ifstream queryFile(queryFileName);
    ASSERT_EQ(queryFile.is_open(), true);
    string q;
    queryFile >> q;
    queryFile.close();
    aod.execute(q, qr);

    // Compare the actual result with the expected result
    string expectedOutputFileName = dataDir + testName + "_expected_output.txt";
    compareExecuteResult(expectedOutputFileName, csrPtr.get(), qr.csrPtr, false);

    AndOrDag aodRev;    // Test right-to-left, no loop caching execution
    aodRev.setCsrPtr(csrPtr);
    if (testName == "ConcatTest")
        buildAndOrDagFromFile(aodRev, inputFileName, true, false);
    else
        buildAndOrDagFromFile(aodRev, inputFileName, false, false);
    aodRev.initAuxiliary();
    if (mat) {
        for (size_t i : matIdx)
            aodRev.setMaterialized(i);
        aodRev.materialize();
    }
    QueryResult qrRev(nullptr, false);
    aodRev.execute(q, qrRev);

    // Compare the actual result with the expected result
    compareExecuteResult(expectedOutputFileName, csrPtr.get(), qrRev.csrPtr, false);
}

TEST_P(ExecuteTestSuite, NfaExecuteTest) {
    const auto &pr = GetParam();
    const string &testName = pr.first;  // Same procedure whether materialize or not
    string queryFileName = dataDir + testName + "_query.txt";
    std::ifstream queryFile(queryFileName);
    ASSERT_EQ(queryFile.is_open(), true);
    string q;
    queryFile >> q;
    queryFile.close();
    Rpq2NFAConvertor cvrt;
    shared_ptr<NFA> dfaPtr = cvrt.convert(q)->convert2Dfa();
    shared_ptr<MappedCSR> res = dfaPtr->execute(csrPtr);
    // Compare the actual result with the expected result
    string expectedOutputFileName = dataDir + testName + "_expected_output.txt";
    compareExecuteResult(expectedOutputFileName, csrPtr.get(), res.get(), true);
}

std::vector<std::string> executeTestNames({"SingleIriTest", "SingleInverseIriTest", "AlternationTest", "ConcatTest",
"ConcatKleeneTest", "KleeneIriConcatTest", "KleeneStarIriConcatTest", "IriKleeneStarConcat"});
std::vector<std::pair<std::string, bool>> genExecuteTestNamesWithMode() {
    std::vector<std::pair<std::string, bool>> ret;
    for (const auto &testName : executeTestNames) {
        ret.emplace_back(testName, true);
        ret.emplace_back(testName, false);
    }
    return ret;
}
INSTANTIATE_TEST_SUITE_P(ExecuteTestSuiteInstance, ExecuteTestSuite, ::testing::ValuesIn(genExecuteTestNamesWithMode()));