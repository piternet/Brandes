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

map< int, vector<int> > graph;
set<int> vertices;
map<int, double> betweenness; // WSPOLDZIELONA
mutex guard;
mutex pendingGuard;
queue<int> pending;

void doBrandes(int s) {
	stack<int> S;
	map<int, vector<int> > P;
	map<int, int> sigma;
	map<int, int> d;
	map<int, double> delta;

	for(const auto &w : vertices) {
		sigma[w] = 0;
		d[w] = -1;
		delta[w] = 0.0;
	}

	sigma[s] = 1;
	d[s] = 0;
	queue<int> Q;
	Q.push(s);
	while(!Q.empty()) {
		int v = Q.front();
		Q.pop();
		S.push(v);
		for(const auto w : graph[v]) {
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
			guard.lock();
			betweenness[w] += delta[w];
			guard.unlock();
		}
	}
}

void recurBrandes() {
	pendingGuard.lock();
	if(pending.empty()) {
            pendingGuard.unlock();
            return;
        }
	int v = pending.front();
	pending.pop();
        pendingGuard.unlock();
	doBrandes(v);
	recurBrandes();
}

void foo() {
    return;
}

void brandes(int numberOfThreads) {
	for(const auto v : vertices) {
		betweenness[v] = 0.0;
		pending.push(v);
	}
	vector<thread> threads;
	for(int i=0; i<numberOfThreads; i++) {
		threads.push_back(thread(recurBrandes));
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

	while(!input.eof()) {
		int u, v;
		input >> u >> v;
		if(input.eof()) break;
		graph[u].push_back(v);
		vertices.insert(u);
		vertices.insert(v);
	}
	input.close();

	brandes(numberOfThreads);

	for(const auto &p : betweenness) {
		if(graph[p.first].size() > 0)
			output << p.first << " " << p.second << endl;
	}
	output.close();
	return 0;
}