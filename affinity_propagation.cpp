#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <string.h>
using namespace std;

//N is the number of two-dimension data points
//S is the similarity matrix
//R is the responsibility matrix
//A is the availabiltiy matrix
//iter is the maximum number of iterations
//lambda is the damping factor
int N = 25;

void readS_rpq(double **S, const vector<vector<int>> &labelLists) {
	int size = N*(N-1)/2;
	vector<double> tmpS;
	//compute similarity between data point i and j (i is not equal to j)
	//similarity definition: the number of common labels / size of unioned label set
	for(int i=0; i<N-1; i++) {
		for(int j=i+1; j<N; j++) {
			int common = 0;
			for (int k = 0, l = 0; k < labelLists[i].size() && l < labelLists[j].size();) {
				if (labelLists[i][k] == labelLists[j][l]) {
					common++;
					k++;
					l++;
				} else if (labelLists[i][k] < labelLists[j][l]) {
					k++;
				} else {
					l++;
				}
			}
			S[i][j] = double(common) / (double)(labelLists[i].size() + labelLists[j].size() - common);
			S[j][i] = S[i][j];
			tmpS.push_back(S[i][j]); 
		}
	}
	//compute preferences for all data points: median 
	sort(tmpS.begin(), tmpS.end());
	double median = 0;
	
	if(size%2==0) 
		median = (tmpS[size/2]+tmpS[size/2-1])/2;
	else 
		median = tmpS[size/2];
	for(int i=0; i<N; i++) S[i][i] = median;
}

const char* dataFileName = "ToyProblemData.txt";
void readS(double **S, const char* dfn) {
	//read data 
	ifstream myfile(dfn);
	
	double dataPoint[N][2] = {0};
	for(int i=0; i<N; i++) {
		myfile >> dataPoint[i][0] >> dataPoint[i][1];
	}
	myfile.close();
	
	int size = N*(N-1)/2;
	vector<double> tmpS;
	//compute similarity between data point i and j (i is not equal to j)
	for(int i=0; i<N-1; i++) {
		for(int j=i+1; j<N; j++) {
			S[i][j] = -((dataPoint[i][0]-dataPoint[j][0])*(dataPoint[i][0]-dataPoint[j][0])+(dataPoint[i][1]-dataPoint[j][1])*(dataPoint[i][1]-dataPoint[j][1]));
			S[j][i] = S[i][j];
			tmpS.push_back(S[i][j]); 
		}
	}
	//compute preferences for all data points: median 
	sort(tmpS.begin(), tmpS.end());
	double median = 0;
	
	if(size%2==0) 
		median = (tmpS[size/2]+tmpS[size/2-1])/2;
	else 
		median = tmpS[size/2];
	for(int i=0; i<N; i++) S[i][i] = median;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		cout << "Usage: " << argv[0] << " <num_iter> <damping_factor>" << endl;
		return 1;
	}
	// int iter = 10;
	// double lambda = 0.6;
	int iter = atoi(argv[1]);
	double lambda = atof(argv[2]);
	// cout << "iter: " << iter << " lambda: " << lambda << endl;

	// Read workload queries
	string queryFilePath = "../rpq-view/real_data/wikidata/queries.txt";
    ifstream fin(queryFilePath);
	unordered_set<string> qSet;
	vector<string> qVec;
    string q, labelStr;
	size_t lBracPos = 0, rBracPos = 0;
	// Convert regexs into ordered label lists
	vector<vector<int>> labelLists;
	int label = 0;
    while (fin >> q) {
		if (qSet.find(q) != qSet.end())
			continue;
		qSet.emplace(q);
		qVec.emplace_back(q);
		labelLists.emplace_back();
		lBracPos = 0;
		rBracPos = 0;
		while (true) {
			lBracPos = q.find('<', rBracPos);
			if (lBracPos == string::npos)
				break;
			rBracPos = q.find('>', lBracPos);
			if (rBracPos == string::npos)
				break;
			labelStr = q.substr(lBracPos + 1, rBracPos - lBracPos - 1);
			if (labelStr[labelStr.size() - 1] == '-') {
				labelStr.pop_back();
				labelStr = "-" + labelStr;
			}
			label = stoi(labelStr);
			if (find(labelLists.back().begin(), labelLists.back().end(), label) == labelLists.back().end())
				labelLists.back().emplace_back(label);
		}
		sort(labelLists.back().begin(), labelLists.back().end());
	}
	N = qSet.size();
	qSet.clear();

	// double S[N][N] = {0};
	// double R[N][N] = {0};
	// double A[N][N] = {0};
	double **S, **R, **A;
	S = new double*[N];
	R = new double*[N];
	A = new double*[N];
	for (size_t i = 0; i < N; i++) {
		S[i] = new double[N];
		R[i] = new double[N];
		A[i] = new double[N];
		memset(S[i], 0, sizeof(double) * N);
		memset(R[i], 0, sizeof(double) * N);
		memset(A[i], 0, sizeof(double) * N);
	}

	readS_rpq(S, labelLists);
	// readS(S, dataFileName);
	
	auto start_time = std::chrono::steady_clock::now();
	for(int m=0; m<iter; m++) {
	//update responsibility
		for(int i=0; i<N; i++) {
			for(int k=0; k<N; k++) {
				double max = -1e100;
				for(int kk=0; kk<k; kk++) {
					if(S[i][kk]+A[i][kk]>max) 
						max = S[i][kk]+A[i][kk];
				}
				for(int kk=k+1; kk<N; kk++) {
					if(S[i][kk]+A[i][kk]>max) 
						max = S[i][kk]+A[i][kk];
				}
				R[i][k] = (1-lambda)*(S[i][k] - max) + lambda*R[i][k];
			}
		}
	//update availability
		for(int i=0; i<N; i++) {
			for(int k=0; k<N; k++) {
				if(i==k) {
					double sum = 0.0;
					for(int ii=0; ii<i; ii++) {
						sum += max(0.0, R[ii][k]);
					}
					for(int ii=i+1; ii<N; ii++) {
						sum += max(0.0, R[ii][k]);
					}
					A[i][k] = (1-lambda)*sum + lambda*A[i][k];
				} else {
					double sum = 0.0;
					int maxik = max(i, k);
					int minik = min(i, k);
					for(int ii=0; ii<minik; ii++) {
						sum += max(0.0, R[ii][k]);
					}
					for(int ii=minik+1; ii<maxik; ii++) {
						sum += max(0.0, R[ii][k]);
					}
					for(int ii=maxik+1; ii<N; ii++) {
						sum += max(0.0, R[ii][k]);
					}
					A[i][k] = (1-lambda)*min(0.0, R[k][k]+sum) + lambda*A[i][k];
				}
			}
		}
	}
	
	//find the exemplar
	double E[N][N] = {0};
	vector<int> center;
	for(int i=0; i<N; i++) {
		E[i][i] = R[i][i] + A[i][i];
		if(E[i][i]>0) {
			center.push_back(i);
		}
	}
	//data point assignment, idx[i] is the exemplar for data point i
	auto end_time = std::chrono::steady_clock::now();
	std::chrono::microseconds elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "AP time: " << elapsed_microseconds.count() << " us" << std::endl;

	// int idx[N] = {0};
	// TODO: sort queries into clusters, output each cluster
	unordered_map<int, vector<string>> clusterMap;
	for(int i=0; i<N; i++) {
		int idxForI = 0;
		double maxSim = -1e100;
		for(int j=0; j<center.size(); j++) {
			int c = center[j];
			if (S[i][c]>maxSim) {
				maxSim = S[i][c];
				idxForI = c;
			}
		}
		// idx[i] = idxForI;
		clusterMap[idxForI].emplace_back(qVec[i]);
	}
	//output the assignment
	// for(int i=0; i<N; i++) {
	// 	//since the index of data points starts from zero, I add 1 to let it start from 1
	// 	cout << qVec[i] << " " << idx[i]+1 << endl; 
	// }
	for (auto &kv : clusterMap) {
		cout << kv.second.size() << endl;
		for (auto &q : kv.second)
			cout << q << " ";
		cout << endl;
	}
}


