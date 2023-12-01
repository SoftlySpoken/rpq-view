/**
 * @file chooseMatViewsTheoCompare.cpp
 * @author Yue Pang 
 * @brief Compare the theoretical (estimated) workload cost betweewn different view selection methods
 * @date 2023-09-11
 */

#include "AndOrDag.h"
using namespace std;

int main(int argc, char **argv) {
    // Read workload queries
    string dataDir = "../real_data/";
    string graphName = "wikidata";
    // string graphName = "example";
    string queryFilePath = dataDir + graphName + "/queries.txt";
    ifstream fin(queryFilePath);
    unordered_map<string, size_t> q2freq;
    unordered_map<string, size_t>::iterator it;
    string line, q;
    while (fin >> q) {
        it = q2freq.find(q);
        if (it == q2freq.end())
            q2freq[q] = 1;
        else
            it->second++;
    }
    size_t numModes = 5;
    size_t usedSpace = 0, budget = 1000000;
    bool execute = false;
    if (argc == 2 && (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--execute") == 0)) {
        cout << "Execute mode." << endl;
        execute = true;
    }
    // QueryResult qr(nullptr, false);
    float naiveTime = 0;
    vector<float> viewTimeVec(numModes, 0);

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
    
    // Construct DAG and plan
    AndOrDag aod(csrPtr);
    for (const auto &p: q2freq)
        aod.addWorkloadQuery(p.first, p.second);
    aod.initAuxiliary();
    aod.annotateLeafCostCard();
    start_time = std::chrono::steady_clock::now();
    aod.plan();
    end_time = std::chrono::steady_clock::now();
    elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Plan time: " << elapsed_microseconds.count() / 1000.0 << " ms" << std::endl;
    if (execute) {
        for (const auto &p: q2freq) {
            QueryResult qr(nullptr, false);
            start_time = std::chrono::steady_clock::now();
            aod.execute(p.first, qr);
            end_time = std::chrono::steady_clock::now();
            elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            // std::cout << p.first << " " << elapsed_microseconds.count() << std::endl;
            naiveTime += elapsed_microseconds.count() * float(p.second);
            if (qr.newed)
                delete qr.csrPtr;
        }
    }
    std::cout << "Naive execution time: " << naiveTime << " us" << std::endl;

    // Choose materialized views
    float curCostReduction = 0;
    vector<size_t> modesVec({5});
    for (size_t i : modesVec) {
    // for (size_t i = 1; i < numModes; i++) {
        AndOrDag tmpAod(aod);
        start_time = std::chrono::steady_clock::now();
        curCostReduction = tmpAod.chooseMatViews(i, usedSpace, budget);
        end_time = std::chrono::steady_clock::now();
        elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "Choose materialized views time: " << elapsed_microseconds.count() << " us" << std::endl;
        // For each selection method, get the overall cost reduction; print the selected views and the cost reduction
        cout << i << " " << (unsigned long long)(curCostReduction) << " " << usedSpace << endl;
        const auto &q2idx = tmpAod.getQ2idx();
        size_t numMatViews = 0;
        for (const auto &pr : q2idx) {
            if (tmpAod.isMaterialized(pr.second) && !tmpAod.getNodes()[pr.second].getChildIdx().empty()) {
                cout << pr.first << " ";
                numMatViews++;
            }
        }
        cout << endl << numMatViews << endl;
        if (execute) {
            start_time = std::chrono::steady_clock::now();
            tmpAod.materialize();
            end_time = std::chrono::steady_clock::now();
            elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            std::cout << "Materialize views time: " << elapsed_microseconds.count() << " us" << std::endl;
            for (const auto &p: q2freq) {
                QueryResult qr(nullptr, false);
                start_time = std::chrono::steady_clock::now();
                tmpAod.execute(p.first, qr);
                end_time = std::chrono::steady_clock::now();
                elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                std::cout << p.first << " " << elapsed_microseconds.count() << std::endl;
                viewTimeVec[i] += elapsed_microseconds.count() * float(p.second);
                if (qr.newed)
                    delete qr.csrPtr;
            }
            std::cout << "Mode " << i << " execution time: " << viewTimeVec[i] << " us" << std::endl;
        }
    }
}