#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <queue>
#include <stack>
#include <mutex>
#include <thread>

using namespace std;

vector<vector<int>> graph;
vector<int> vertices;
vector<double> betweenness; // WSPOLDZIELONY
map<int, int> translator;
mutex pendingGuard;
queue<int> pending;
int numberOfVertices;

void doBrandes(int s, vector<mutex> &guards) {
	stack<int> S;
	vector<vector<int>>P(numberOfVertices);
	vector<int> sigma(numberOfVertices, 0);
	vector<int> d(numberOfVertices, -1);
	vector<double> delta(numberOfVertices, 0.0);

	sigma[s] = 1;
	d[s] = 0;
	queue<int> Q;
	Q.push(s);
	while(!Q.empty()) {
		int v = Q.front();
		Q.pop();
		S.push(v);
		for(const auto &w : graph[v]) {
			if(d[w] < 0) {
				Q.push(w);
				d[w] = d[v] + 1;
			}
			if(d[w] == d[v] + 1) {
				sigma[w] += sigma[v];
				P[w].push_back(v);
			}
		}
	}

	while(!S.empty()) {
		int w = S.top();
		S.pop();
		for(const auto &v : P[w]) {
			delta[v] += ((double) sigma[v] / (double) sigma[w]) * (1.0 + delta[w]);
		}
		if(w != s) {
			guards[w].lock();
			betweenness[w] += delta[w];
			guards[w].unlock();
		}
	}
}

void whileBrandes(vector<mutex> &guards) {
    while(true) {
	pendingGuard.lock();
	if(pending.empty()) {
            pendingGuard.unlock();
            return;
        }
	int v = pending.front();
	pending.pop();
        pendingGuard.unlock();
	doBrandes(v, guards);
    }
}


void brandes(int numberOfThreads) {
	for(int v=0; v<numberOfVertices; v++) {
		betweenness.push_back(0.0);
		pending.push(v);
	}
        vector<mutex> guards(numberOfVertices);
	vector<thread> threads;
	for(int i=0; i<numberOfThreads; i++) {
		threads.push_back(thread{[&guards]{whileBrandes(guards);}});
	}
	for(int i=0; i<numberOfThreads; i++) {
            if(threads[i].joinable())
                threads[i].join();
	}
}

int main(int argc, char *argv[]) {
	string strThreads(argv[1]), inputFileName(argv[2]), outputFileName(argv[3]);
	int numberOfThreads = stoi(strThreads);
	
	ifstream input(inputFileName);
	ofstream output(outputFileName);
        int freeIndex = 0;
        vector<pair<int, int>> edges;
	while(!input.eof()) {
		int u, v;
		input >> u >> v;
                if(translator.find(u) == translator.end()) {
                    translator[u] = freeIndex;
                    freeIndex++;
                    vertices.push_back(u);
                }
                if(translator.find(v) == translator.end()) {
                    translator[v] = freeIndex;
                    freeIndex++;
                   vertices.push_back(v);
                }
                edges.push_back(make_pair(u, v));
		if(input.eof()) break;
                
	}
	input.close();

        numberOfVertices = freeIndex;
        graph.resize(freeIndex);
        for(const auto &p : edges) {
            int u = p.first, v = p.second;
            graph[translator[u]].push_back(translator[v]);
        }

	brandes(numberOfThreads);

	for(int i=0; i<freeIndex; i++) {
		if(graph[i].size() > 0)
			output << vertices[i] << " " << betweenness[i] << endl;
	}
	output.close();
	return 0;
}
