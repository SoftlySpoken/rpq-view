#include "AndOrDag.h"
#include "Rpq2NFAConvertor.h"
using namespace std;

int main() {
    string dataDir = "../real_data/wikidata/";
    string graphFilePath = dataDir + "graph.txt";
    shared_ptr<MultiLabelCSR> csrPtr = make_shared<MultiLabelCSR>();
    csrPtr->loadGraph(graphFilePath, spo);
    // csrPtr->fillStats();

    string queryFilePath = dataDir + "queries.txt";
    unordered_map<string, size_t> querySet;
    unordered_map<string, size_t>::iterator it;
    ifstream queryFile(queryFilePath);
    string q;
    while (getline(queryFile, q)) {
        it = querySet.find(q);
        if (it == querySet.end())
            querySet[q] = 1;
        else
            it->second++;
    }

    AndOrDag aod(csrPtr);
    for (const auto &pr : querySet)
        aod.addWorkloadQuery(pr.first, pr.second);
    aod.initAuxiliary();
    aod.annotateLeafCostCard();
    aod.plan();

    Rpq2NFAConvertor cvrt;
    std::chrono::_V2::steady_clock::time_point start_time, end_time;
    std::chrono::microseconds elapsed_microseconds;
    for (const auto &pr : querySet) {
        cout << pr.first << endl;

        start_time = std::chrono::steady_clock::now();
        QueryResult qr(nullptr, false);
        aod.execute(pr.first, qr);
        end_time = std::chrono::steady_clock::now();
        elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "AND-OR DAG execution used: " << elapsed_microseconds.count() << " ms" << std::endl;
        if (qr.newed)
            delete qr.csrPtr;

        start_time = std::chrono::steady_clock::now();
        shared_ptr<NFA> dfaPtr = cvrt.convert(pr.first)->convert2Dfa();
        shared_ptr<MappedCSR> res = dfaPtr->execute(csrPtr);
        end_time = std::chrono::steady_clock::now();
        elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "DFA execution used: " << elapsed_microseconds.count() << " ms" << std::endl;
    }
}