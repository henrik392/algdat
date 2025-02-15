#include <chrono>
#include <fstream>
#include <iostream>
#include <locale>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

#define pii pair<int, int>
#define pdd pair<double, double>

class Map {
private:
  string nodesFile, edgesFile, interestPointsFile;

  void readNodes() {
    std::locale::global(std::locale::classic());
    ifstream file(nodesFile);
    if (!file) {
      cout << "Error opening file: " << nodesFile << endl;
      return;
    }

    file >> n;

    nodes.resize(n);
    edges.resize(n);
    inverseEdges.resize(n);

    for (int i = 0; i < n; i++) {
      int id;
      double latitude, longitude;
      file >> id >> latitude >> longitude;
      nodes[i] = {latitude, longitude};
      // cout << "Node: " << i << " Latitude: " << latitude << " Longitude: " << longitude << endl;
    }

    file.close();
  }

  void readEdges() {
    ifstream file(edgesFile);
    if (!file) {
      cout << "Error opening file: " << edgesFile << endl;
      return;
    }

    file >> m;

    for (int i = 0; i < m; i++) {
      // Tid i hundredels sekund
      int from, to, time, length, speedLimit;
      file >> from >> to >> time >> length >> speedLimit;
      edges[from].push_back({to, time});
      inverseEdges[to].push_back({from, time});
    }

    file.close();
  }

  void readInterestPoints() {
    ifstream file(interestPointsFile);
    if (!file) {
      cout << "Error opening file: " << interestPointsFile << endl;
      return;
    }

    file >> k;
    for (int i = 0; i < k; i++) {
      int node, interest;
      string name, tmp;

      file >> node >> interest;
      file >> ws; // Ignore any leading whitespace

      // Read the quote and the name
      getline(file, tmp, '"');  // Read up to the first quote
      getline(file, name, '"'); // Read the name inside quotes

      interestPoints[node] = {interest, name};
      nameTointerestPoint[name] = {node, interest};
    }

    file.close();
  }

  void readMap() {
    readNodes();
    readEdges();
    readInterestPoints();
  }

public:
  unordered_map<int, pair<int, string>> interestPoints; // id -> interest + name
  unordered_map<string, pii> nameTointerestPoint;       // name -> id + interest
  vector<vector<pii>> inverseEdges;
  vector<vector<pii>> edges;
  vector<pdd> nodes;
  int n, m, k;

  Map(string nodesFile, string edgesFile, string interestPointsFile) : nodesFile(nodesFile), edgesFile(edgesFile), interestPointsFile(interestPointsFile) {
    readMap();
  }
};

vector<int> reconstructPath(int start, int end, unordered_map<int, int> &predecessors) {
  vector<int> path;
  for (int at = end; at != start; at = predecessors[at]) {
    path.push_back(at);
    if (predecessors.find(at) == predecessors.end()) {
      return {};
    }
  }
  path.push_back(start);
  reverse(path.begin(), path.end());
  return path;
}

pair<int, vector<int>> djikstras(Map &map, int start, int end) {
  unordered_map<int, int> predecessors;
  unordered_set<int> visited;

  priority_queue<pii, vector<pii>, greater<pii>> pq;
  pq.push({0, start});

  int count = 0;

  while (!pq.empty()) {
    count++;
    pii current = pq.top(); // {time, node}
    pq.pop();

    if (visited.find(current.second) != visited.end()) {
      continue;
    }

    visited.insert(current.second);

    if (current.second == end) {
      cout << "Number of nodes visited: " << count << endl;
      return {current.first, reconstructPath(start, end, predecessors)};
    }

    for (pii edge : map.edges[current.second]) {
      int nextNode = edge.first;
      int newDist = current.first + edge.second;

      if (visited.find(nextNode) == visited.end()) {
        pq.push({newDist, nextNode});
        predecessors[nextNode] = current.second; // Update predecessor
      }
    }
  }

  return {-1, {}};
}

// vector<string> LANDMARKS = {"Kristiansand", "Stavanger", "Bergen", "Trondheim", "Tromsø", "Oslo"};

void writeCoordintePathFromLandmarksToFile(Map &map, string startName, string endName, string outputFileName) {
  ofstream file(outputFileName);
  if (!file) {
    cout << "Error opening file: "
         << outputFileName << endl;
    return;
  }

  cout << "Finding path from " << startName << " to " << endName << "..." << endl;

  int start = map.nameTointerestPoint[startName].first;
  int end = map.nameTointerestPoint[endName].first;

  auto start_time = chrono::high_resolution_clock::now();
  auto result = djikstras(map, start, end);
  auto end_time = chrono::high_resolution_clock::now();

  int time = result.first;
  int seconds = time / 100;
  int minutes = seconds / 60;
  int hours = minutes / 60;
  minutes %= 60;
  seconds %= 60;

  cout << "Time taken for algorithm to run was : " << chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() << "ms" << endl;
  cout << "Time from " << startName << " to " << endName << " is " << hours << " hours, " << minutes << " minutes, " << seconds << " seconds." << endl;

  file << "latitude,longitude\n";

  int MAXWAYPOINTS = 100;
  int step = result.second.size() / MAXWAYPOINTS;
  if (step == 0) {
    step = 1;
  }
  for (size_t i = 0; i < result.second.size(); i += step) {
    int node = result.second[i];
    file << map.nodes[node].first << "," << map.nodes[node].second << "\n";
  }

  // Include the last waypoint if it's not already included
  if ((result.second.size() - 1) % step != 0) {
    int node = result.second.back();
    file << map.nodes[node].first << "," << map.nodes[node].second << "\n";
  }

  file.close();

  cout << "Done writing to file." << endl;
}

int main() {
  cout << "Reading file..." << endl;
  string pathToMap = "oving9/data/norden";
  Map map(pathToMap + "/noder.txt", pathToMap + "/kanter.txt", pathToMap + "/interessepkt.txt");
  cout << "Done reading file." << endl;

  writeCoordintePathFromLandmarksToFile(map, "Orkanger", "Trondheim", "oving9/data/route1.csv");

  return 0;
}