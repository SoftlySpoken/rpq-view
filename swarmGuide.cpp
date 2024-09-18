/**
 * @file matMostFrequent.cpp
 * @author Yue Pang 
 * @brief Materialize the most frequent workload queries' results until the actual memory usage
 * exceeds a threshold (determined by human inspecting the top console) - 192 GB
 * @date 2023-10-10
*/
#include "AndOrDag.h"
using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
		cout << "Usage: " << argv[0] << " <num_iter> <damping_factor>" << endl;
		return 1;
	}
    int num_iter = atoi(argv[1]);
	float damping_factor = atof(argv[2]);

    string inputFilePath;
    ifstream inputFile;

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

    // Read the ap views
    vector<string> apViews;
    string apPath = "ap_output/";
    string apFilePath = apPath + "ap_" + to_string(num_iter) + "_" + to_string(damping_factor) + "-view-manual.txt";
    ifstream apFile(apFilePath);
    int clusterSz;
    while (apFile >> clusterSz) {
        if (clusterSz < 0)
            getline(apFile, q);
        else if (clusterSz == 0)
            continue;
        else {
            apFile >> clusterSz;
            for (size_t i = 0; i < clusterSz; i++)
                apFile >> q;
            apFile >> q;
            if (q[0] != 'N')
                apViews.emplace_back(q);
        }
    }

    // Materialize the ap views
    float matTime = 0;
    size_t curIdx = 0;
    for (const string &v : apViews) {
        it = aod.getQ2idx().find(v);
        if (it == aod.getQ2idx().end())
            continue;
        curIdx = it->second;
        start_time = std::chrono::steady_clock::now();
        aod.executeNode(curIdx, aod.getNodes()[curIdx].getRes(), nullptr, nullptr, nullptr, curIdx);
        end_time = std::chrono::steady_clock::now();
        elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        aod.getMaterialized()[curIdx] = true;
        matTime += elapsed_microseconds.count();
    }

    // Execute the workload queries
    float queryTime = 0;
    for (const auto &p: q2freq) {
        QueryResult qr(nullptr, false);
        start_time = std::chrono::steady_clock::now();
        aod.execute(p.first, qr);
        end_time = std::chrono::steady_clock::now();
        elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << elapsed_microseconds.count() << std::endl;
        queryTime += elapsed_microseconds.count() * float(p.second);
    }
    cout << "Total time: " << queryTime << " us" << endl;
}