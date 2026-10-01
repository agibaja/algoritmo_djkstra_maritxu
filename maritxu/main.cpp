#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;
using Clock = chrono::high_resolution_clock;

struct Edge {
    int to;
    int weight;
};

struct QueryResult {
    bool reachable = false;
    long long cost = 0;
    vector<int> path;
    double milliseconds = 0.0;
};

struct NegativeEdgeInfo {
    bool found = false;
    int fromId = 0;
    int toId = 0;
    int weight = 0;
};

static string nodeName(int id, const unordered_map<int, string>& names) {
    auto it = names.find(id);
    if (it == names.end()) return to_string(id);
    return it->second;
}

static QueryResult dijkstra(int sourceIndex, int targetIndex,
    const vector<vector<Edge>>& graph,
    const vector<int>& indexToId) {
    auto queryStart = Clock::now();

    QueryResult result;

    if (sourceIndex == targetIndex) {
        result.reachable = true;
        result.cost = 0;
        result.path.push_back(indexToId[sourceIndex]);

        auto queryEnd = Clock::now();
        result.milliseconds =
            chrono::duration<double, milli>(queryEnd - queryStart).count();

        return result;
    }

    constexpr const long long INF = numeric_limits<long long>::max() / 4;
    const int n = static_cast<int>(graph.size());

    vector<long long> dist(n, INF);
    vector<int> parent(n, -1);

    priority_queue<pair<long long, int>,
        vector<pair<long long, int>>,
        greater<pair<long long, int>>> pq;

    dist[sourceIndex] = 0;
    pq.push({ 0, sourceIndex });

    while (!pq.empty()) {
        pair<long long, int> actual = pq.top();
        pq.pop();

        long long currentDist = actual.first;
        int u = actual.second;

        if (currentDist != dist[u]) continue;
        if (u == targetIndex) break;

        for (const Edge& edge : graph[u]) {
            int v = edge.to;
            long long newDist = currentDist + edge.weight;

            if (newDist < dist[v]) {
                dist[v] = newDist;
                parent[v] = u;
                pq.push({ newDist, v });
            }
        }
    }

    if (dist[targetIndex] != INF) {
        result.reachable = true;
        result.cost = dist[targetIndex];

        vector<int> reversedPath;
        for (int at = targetIndex; at != -1; at = parent[at]) {
            reversedPath.push_back(indexToId[at]);
        }

        reverse(reversedPath.begin(), reversedPath.end());
        result.path = reversedPath;
    }

    auto queryEnd = Clock::now();
    result.milliseconds =
        chrono::duration<double, milli>(queryEnd - queryStart).count();

    return result;
}

int main(int argc, char* argv[]) {
    auto programStart = Clock::now();

    if (argc != 2) {
        cerr << "Uso: ./maritxu grafo.txt\n";
        return 1;
    }

    ifstream input(argv[1]);

    if (!input.is_open()) {
        cerr << "ERROR: No se pudo abrir el fichero de entrada: "
            << argv[1] << "\n";
        cerr << "El archivo debe existir y estar cerrado para que el programa pueda leerlo.\n";
        return 1;
    }

    ofstream output("resultados.txt");
    if (!output) {
        cerr << "ERROR: No se pudo crear resultados.txt.\n";
        return 1;
    }

    int n = 0;
    int m = 0;
    input >> n >> m;

    unordered_map<int, string> names;
    unordered_map<int, int> idToIndex;
    vector<int> indexToId;

    names.reserve(static_cast<size_t>(n) * 2);
    idToIndex.reserve(static_cast<size_t>(n) * 2);
    indexToId.reserve(n);

    for (int i = 0; i < n; ++i) {
        int id;
        string name;

        input >> id >> name;

        names[id] = name;
        idToIndex[id] = i;
        indexToId.push_back(id);
    }

    vector<vector<Edge>> graph(n);
    NegativeEdgeInfo negative;

    for (int i = 0; i < m; ++i) {
        int fromId;
        int toId;
        int weight;

        input >> fromId >> toId >> weight;

        if (weight < 0 && !negative.found) {
            negative.found = true;
            negative.fromId = fromId;
            negative.toId = toId;
            negative.weight = weight;
        }

        auto fromIt = idToIndex.find(fromId);
        auto toIt = idToIndex.find(toId);

        if (fromIt != idToIndex.end() && toIt != idToIndex.end()) {
            int fromIndex = fromIt->second;
            int toIndex = toIt->second;

            graph[fromIndex].push_back({ toIndex, weight });
            graph[toIndex].push_back({ fromIndex, weight });
        }
    }

    auto loadEnd = Clock::now();
    double loadMilliseconds =
        chrono::duration<double, milli>(loadEnd - programStart).count();

    if (negative.found) {
        output << "ERROR: El grafo contiene aristas con peso negativo. "
            "Dijkstra no es aplicable.\n";

        output << "Arista con peso negativo detectada: "
            << nodeName(negative.fromId, names)
            << " -- "
            << nodeName(negative.toId, names)
            << " (peso: "
            << negative.weight
            << ")\n";

        return 0;
    }

    string marker;
    input >> marker;

    vector<pair<int, int>> queries;
    int sourceId;
    int targetId;

    while (input >> sourceId >> targetId) {
        queries.push_back({ sourceId, targetId });
    }

    double processingMilliseconds = 0.0;
    output << fixed << setprecision(3);

    for (size_t i = 0; i < queries.size(); ++i) {
        int originId = queries[i].first;
        int destinationId = queries[i].second;

        output << "Consulta " << (i + 1) << ": "
            << nodeName(originId, names)
            << " -> "
            << nodeName(destinationId, names)
            << "\n";

        auto originIt = idToIndex.find(originId);
        auto destinationIt = idToIndex.find(destinationId);

        QueryResult result;

        if (originIt != idToIndex.end() && destinationIt != idToIndex.end()) {
            result = dijkstra(originIt->second,
                destinationIt->second,
                graph,
                indexToId);
        }
        else {
            auto queryStart = Clock::now();
            auto queryEnd = Clock::now();

            result.reachable = false;
            result.milliseconds =
                chrono::duration<double, milli>(queryEnd - queryStart).count();
        }

        processingMilliseconds += result.milliseconds;

        if (result.reachable) {
            output << "Coste: " << result.cost << "\n";
            output << "Camino: ";

            for (size_t j = 0; j < result.path.size(); ++j) {
                if (j > 0) output << " -> ";
                output << nodeName(result.path[j], names);
            }

            output << "\n";
        }
        else {
            output << "Coste: INFINITO\n";
            output << "Camino: SIN CAMINO\n";
        }

        output << "Tiempo: " << result.milliseconds << " ms\n";

        if (i + 1 < queries.size()) {
            output << "\n";
        }
    }

    auto programEnd = Clock::now();
    double totalMilliseconds =
        chrono::duration<double, milli>(programEnd - programStart).count();

    if (!queries.empty()) {
        output << "\n";
    }

    output << "---\n";
    output << "Tiempo de carga de datos: "
        << loadMilliseconds
        << " ms\n";

    output << "Tiempo de procesamiento de consultas: "
        << processingMilliseconds
        << " ms\n";

    output << "Tiempo total: "
        << totalMilliseconds
        << " ms\n";

    return 0;
}