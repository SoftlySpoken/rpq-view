/**
 * @file productGraphQueryBatch.cpp
 * @author Yue Pang (michelle.py@pku.edu.cn)
 * @brief Compute the product graph of the line graphs of a batch of RPQs
 * @date 2023-10-13
 */

#include "Rpq2NFAConvertor.h"
using namespace std;

void getProductGraph(LineGraph &prod, vector<string> queryBatch) {
    Rpq2NFAConvertor cvrt;
    vector<shared_ptr<NFA>> dfaPtrBatch;
    shared_ptr<NFA> curDfaPtr = cvrt.convert(queryBatch[0])->convert2Dfa()->minimizeDfa();
    curDfaPtr->removeSelfLoop();
    curDfaPtr->fillLineGraph();
    dfaPtrBatch.emplace_back(curDfaPtr);
    prod = curDfaPtr->lg;
    size_t qIdx = 1;
    while (qIdx < queryBatch.size()) {
        if (prod.getType() == 0)
            break;
        curDfaPtr = cvrt.convert(queryBatch[qIdx])->convert2Dfa()->minimizeDfa();
        qIdx++;
        curDfaPtr->removeSelfLoop();
        curDfaPtr->fillLineGraph();
        dfaPtrBatch.emplace_back(curDfaPtr);
        prod *= curDfaPtr->lg;
    }
}

int main(int argc, char **argv) {
    // Read batches of queries from the ap output file
    // If the batch only has 1 query, skip
    // Compute the product graph of each batch. (In the chain process, if already no node / no edge / no non-weak edge, terminate early)
    // Output the batch and the product graph status
    if (argc != 2) {
        cout << "Usage: ./productGraphQueryBatch <ap_output_file>" << endl;
        return 0;
    }
    string apOutputFile(argv[1]);
    // string apOutputFile = "../results/test.txt";
    ifstream fin(apOutputFile);
    string tmpStr;
    getline(fin, tmpStr);
    size_t numQuery = 0;
    string curQ;
    while (fin >> numQuery) {
        if (numQuery == 1) {
            fin >> curQ;
            cout << -1 << endl << curQ << endl; // No common (only 1 query in a cluster)
            continue;
        }
        vector<string> queryBatch;
        for (size_t i = 0; i < numQuery; i++) {
            fin >> curQ;
            queryBatch.emplace_back(curQ);
        }
        LineGraph prod;
        getProductGraph(prod, queryBatch);
        int tp = prod.getType();
        cout << tp << endl;
        if (tp == 1 || tp == 2) {
            cout << queryBatch.size() << " ";
            for (const auto &q : queryBatch)
                cout << q << " ";
            cout << endl;   // Output the valid batches
        }
    }
}