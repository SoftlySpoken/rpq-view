/**
 * @file chooseMatViewsTheoCompare.cpp
 * @author Yue Pang (michelle.py@pku.edu.cn)
 * @brief Compare the theoretical (estimated) workload cost betweewn different view selection methods
 * @date 2023-09-11
 */

#include "AndOrDag.h"
using namespace std;

int main() {
    // Read workload queries
    string dataDir = "../real_data/";
    string graphName = "wikidata";
    string queryFilePath = dataDir + graphName + "/queries.txt";
    ifstream fin(queryFilePath);
    unordered_map<string, size_t> q2freq;
    unordered_map<string, size_t>::iterator it;
    size_t pos = 0;
    string line, q;
    while (fin >> q) {
        it = q2freq.find(q);
        if (it == q2freq.end())
            q2freq[q] = 1;
        else
            it->second++;
    }

    // Read graph
    std::shared_ptr<MultiLabelCSR> csrPtr = make_shared<MultiLabelCSR>();
    string graphFilePath = dataDir + graphName + "/graph.txt";
    csrPtr->loadGraph(graphFilePath);
    csrPtr->fillStats();
    
    // Construct DAG and plan
    AndOrDag aod(csrPtr);
    for (auto &p: q2freq)
        aod.addWorkloadQuery(p.first, p.second);
    aod.initAuxiliary();
    aod.annotateLeafCostCard();
    aod.plan();

    // Choose materialized views
    size_t numModes = 5;
    size_t budget = 1000000;
    float curCostReduction = 0;
    for (size_t i = 0; i < numModes; i++) {
        AndOrDag tmpAod(aod);
        curCostReduction = tmpAod.chooseMatViews(i, budget);
        // For each selection method, get the overall cost reduction; print the selected views and the cost reduction
        cout << i << " " << curCostReduction << endl;
        const auto &q2idx = tmpAod.getQ2idx();
        for (const auto &pr : q2idx) {
            if (tmpAod.isMaterialized(pr.second) && !tmpAod.getNodes()[pr.second].getChildIdx().empty())
                cout << pr.first << " ";
        }
        cout << endl;
    }
}