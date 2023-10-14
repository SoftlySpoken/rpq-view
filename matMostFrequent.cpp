/**
 * @file matMostFrequent.cpp
 * @author Yue Pang (michelle.py@pku.edu.cn)
 * @brief Materialize the most frequent workload queries' results until the actual memory usage
 * exceeds a threshold (determined by human inspecting the top console) - 192 GB
 * @date 2023-10-10
*/
#include "AndOrDag.h"
using namespace std;

int main(int argc, char **argv) {
    int exeMode = 0;
    string inputFilePath;
    ifstream inputFile;
    if (argc > 1) {
        if (strcmp(argv[1], "no-probe") == 0) {
            cout << "No probe" << endl;
            exeMode = 1;
        } else if (strcmp(argv[1], "ap") == 0) {
            exeMode = 2;
            if (argc != 3) {
                cout << "Usage: ./matMostFrequent [no-probe / ap] [inputFilePath]" << endl;
                return 0;
            }
            inputFilePath = argv[2];
            cout << "ap" << endl << inputFilePath << endl;
            inputFile.open(inputFilePath);
            if (!inputFile.is_open()) {
                cout << "Cannot open input file" << endl;
                return 0;
            }
        } else {
            cout << "Usage: ./matMostFrequent [no-probe / ap] [inputFilePath]" << endl;
            return 0;
        }
    }

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

    // Sort the workload queries by descending frequency
    vector<pair<string, size_t>> qFreqVec(q2freq.begin(), q2freq.end());
    sort(qFreqVec.begin(), qFreqVec.end(), [](const pair<string, size_t> &a, const pair<string, size_t> &b) {
        return a.second > b.second;
    });

    // Materialize the most frequent workload queries' results until the actual memory usage
    // exceeds a threshold (determined by human inspecting the top console)
    float matTime = 0;
    if (exeMode == 0) {
        string curInput = "";
        size_t i = 0, curIdx = 0;
        do {
            cout << qFreqVec[i].first << endl;
            it = aod.getQ2idx().find(qFreqVec[i].first);
            if (it == aod.getQ2idx().end())
                continue;
            curIdx = it->second;
            start_time = std::chrono::steady_clock::now();
            aod.executeNode(curIdx, aod.getNodes()[curIdx].getRes(), nullptr, nullptr, nullptr, curIdx);
            end_time = std::chrono::steady_clock::now();
            elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            aod.getMaterialized()[curIdx] = true;
            matTime += elapsed_microseconds.count();
            i++;
            cin >> curInput;
        } while (curInput != "STOP" && i < qFreqVec.size());
        if (i < qFreqVec.size()) {
            aod.getMaterialized()[curIdx] = false;  // Tell the DAG not to use the last materialized result causing the memory to exceed
            matTime -= elapsed_microseconds.count();
        }
    } else if (exeMode == 1) {
        ifstream fin("../matMostFrequent.txt");
        string q;
        while (fin >> q) {
            cout << q << endl;
            it = aod.getQ2idx().find(q);
            if (it == aod.getQ2idx().end())
                continue;
            size_t curIdx = it->second;
            start_time = std::chrono::steady_clock::now();
            aod.executeNode(curIdx, aod.getNodes()[curIdx].getRes(), nullptr, nullptr, nullptr, curIdx);
            end_time = std::chrono::steady_clock::now();
            elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            aod.getMaterialized()[curIdx] = true;
            matTime += elapsed_microseconds.count();
        }
    } else if (exeMode == 2) {
        int prodType = -1;
        string q;
        vector<string> qVec;
        while (inputFile >> prodType) {
            if (prodType == -1) {
                inputFile >> q;
                qVec.emplace_back(q);
            } else if (prodType == 1 || prodType == 2) {
                size_t relCnt = 0;
                inputFile >> relCnt;
                for (size_t i = 0; i < relCnt; i++)
                    inputFile >> q;
                inputFile >> q;
                if (q[0] != 'N')
                    qVec.emplace_back(q);
            }
        }
        for (const auto &curQ : qVec) {
            cout << curQ << endl;
            it = aod.getQ2idx().find(curQ);
            if (it == aod.getQ2idx().end())
                continue;
            size_t curIdx = it->second;
            start_time = std::chrono::steady_clock::now();
            aod.executeNode(curIdx, aod.getNodes()[curIdx].getRes(), nullptr, nullptr, nullptr, curIdx);
            end_time = std::chrono::steady_clock::now();
            elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            aod.getMaterialized()[curIdx] = true;
            matTime += elapsed_microseconds.count();
        }
    }
    cout << "Materialize time: " << matTime << " us" << endl;

    // Execute the workload queries
    float queryTime = 0;
    for (const auto &p: qFreqVec) {
        QueryResult qr(nullptr, false);
        start_time = std::chrono::steady_clock::now();
        std::cout << p.first << " ";
        aod.execute(p.first, qr);
        end_time = std::chrono::steady_clock::now();
        elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << elapsed_microseconds.count() << std::endl;
        queryTime += elapsed_microseconds.count() * float(p.second);
    }
    cout << "Total time: " << queryTime << " us" << endl;
}