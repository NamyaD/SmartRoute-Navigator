#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
const double INF = std::numeric_limits<double>::infinity();

// ─────────────────────────────────────────────
//  Traffic levels mapped to multipliers
//  LOW  → 1.0x  (free flow)
//  MED  → 1.5x  (moderate)
//  HIGH → 2.5x  (heavy congestion)
// ─────────────────────────────────────────────
enum class TrafficLevel { LOW, MEDIUM, HIGH };

std::string  trafficToString  (TrafficLevel t);
TrafficLevel stringToTraffic  (const std::string& s);
double       trafficMultiplier(TrafficLevel t);

// ─────────────────────────────────────────────
//  Road  – edge in the graph
// ─────────────────────────────────────────────
struct Road {
    std::string  destination;
    double       distance;     // km
    double       cost;         // Rs.
    TrafficLevel traffic;

    Road(const std::string& dest, double dist, double c, TrafficLevel t)
        : destination(dest), distance(dist), cost(c), traffic(t) {}
};

// ─────────────────────────────────────────────
//  City  – node in the graph
// ─────────────────────────────────────────────
struct City {
    std::string       name;
    std::vector<Road> roads;   // adjacency list

    explicit City(const std::string& n) : name(n) {}
};

// ─────────────────────────────────────────────
//  RouteResult  – returned after any search
// ─────────────────────────────────────────────
struct RouteResult {
    std::vector<std::string> path;
    double totalDistance = 0.0;
    double totalCost     = 0.0;
    double totalTime     = 0.0;   // hours (effective, includes traffic)
    bool   found         = false;

    void print() const;
};

// ─────────────────────────────────────────────
//  Graph  – core data structure
// ─────────────────────────────────────────────
class Graph {
public:
    // ── Mutators ─────────────────────────────
    bool addCity(const std::string& name);
    bool addRoad(const std::string& from,
                 const std::string& to,
                 double distance,
                 double cost,
                 TrafficLevel traffic,
                 bool bidirectional = true);

    // ── Queries ──────────────────────────────
    bool cityExists(const std::string& name) const;
    int  cityCount()  const;
    int  roadCount()  const;
    void displayMap() const;

    // ── Algorithms ───────────────────────────
    // Dijkstra variants
    RouteResult shortestRoute(const std::string& src, const std::string& dst) const;
    RouteResult cheapestRoute(const std::string& src, const std::string& dst) const;
    RouteResult fastestRoute (const std::string& src, const std::string& dst) const;

    // BFS: full reachability from src
    std::vector<std::string> bfsReachable(const std::string& src) const;

    // BFS: actual shortest-hop PATH from src to dst (predecessor map)
    RouteResult bfsShortestPath(const std::string& src, const std::string& dst) const;

    // DFS: full exploration from src
    std::vector<std::string> dfsExplore(const std::string& src) const;

    // File I/O
    bool saveToFile  (const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

private:
    std::unordered_map<std::string, City> cities_;

    // Generic Dijkstra – weight function injected by caller
    RouteResult dijkstra_(const std::string& src,
                          const std::string& dst,
                          std::function<double(const Road&)> weightFn) const;

    void dfsHelper_(const std::string& current,
                    std::set<std::string>& visited,
                    std::vector<std::string>& order) const;
};

#endif // GRAPH_H
