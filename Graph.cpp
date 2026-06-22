#include "Graph.h"

// ─────────────────────────────────────────────
//  Free helper functions
// ─────────────────────────────────────────────
std::string trafficToString(TrafficLevel t) {
    switch (t) {
        case TrafficLevel::LOW:    return "LOW";
        case TrafficLevel::MEDIUM: return "MEDIUM";
        case TrafficLevel::HIGH:   return "HIGH";
    }
    return "LOW";
}

TrafficLevel stringToTraffic(const std::string& s) {
    if (s == "HIGH")   return TrafficLevel::HIGH;
    if (s == "MEDIUM") return TrafficLevel::MEDIUM;
    return TrafficLevel::LOW;
}

double trafficMultiplier(TrafficLevel t) {
    switch (t) {
        case TrafficLevel::LOW:    return 1.0;
        case TrafficLevel::MEDIUM: return 1.5;
        case TrafficLevel::HIGH:   return 2.5;
    }
    return 1.0;
}

// ─────────────────────────────────────────────
//  RouteResult::print
// ─────────────────────────────────────────────
void RouteResult::print() const {
    if (!found || path.empty()) {
        std::cout << "\n  [!] No route found between the selected cities.\n";
        return;
    }

    std::cout << "\n  Route  : ";
    for (size_t i = 0; i < path.size(); ++i) {
        std::cout << path[i];
        if (i + 1 < path.size()) std::cout << "  -->  ";
    }
    std::cout << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Distance : " << totalDistance << " km\n";
    std::cout << "  Cost     : Rs. " << totalCost << "\n";
    std::cout << "  Eff.Time : " << totalTime  << " hrs (incl. traffic factor)\n";
}

// ─────────────────────────────────────────────
//  Graph – Mutators
// ─────────────────────────────────────────────
bool Graph::addCity(const std::string& name) {
    if (cityExists(name)) return false;
    cities_.emplace(name, City(name));
    return true;
}

bool Graph::addRoad(const std::string& from,
                    const std::string& to,
                    double distance,
                    double cost,
                    TrafficLevel traffic,
                    bool bidirectional) {
    if (!cityExists(from) || !cityExists(to)) return false;
    if (distance <= 0 || cost < 0)           return false;

    cities_.at(from).roads.emplace_back(to,   distance, cost, traffic);
    if (bidirectional)
        cities_.at(to).roads.emplace_back(from, distance, cost, traffic);
    return true;
}

// ─────────────────────────────────────────────
//  Graph – Queries
// ─────────────────────────────────────────────
bool Graph::cityExists(const std::string& name) const {
    return cities_.count(name) > 0;
}

int Graph::cityCount() const {
    return static_cast<int>(cities_.size());
}

int Graph::roadCount() const {
    int total = 0;
    for (auto& kv : cities_)
        total += static_cast<int>(kv.second.roads.size());
    return total / 2;
}

void Graph::displayMap() const {
    if (cities_.empty()) {
        std::cout << "  [!] Map is empty. Please add cities first.\n";
        return;
    }

    std::vector<std::string> names;
    names.reserve(cities_.size());
    for (auto& kv : cities_) names.push_back(kv.first);
    std::sort(names.begin(), names.end());

    std::cout << "\n  ══════════════════════════════════════════\n";
    std::cout << "              CURRENT MAP\n";
    std::cout << "  ══════════════════════════════════════════\n";

    for (const auto& name : names) {
        const City& city = cities_.at(name);
        std::cout << "\n  [" << name << "]\n";

        if (city.roads.empty()) {
            std::cout << "      (no roads)\n";
            continue;
        }
        for (const auto& road : city.roads) {
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "      --> " << std::left << std::setw(15) << road.destination
                      << "  | dist: " << std::setw(6) << road.distance << " km"
                      << "  | cost: Rs." << std::setw(7) << road.cost
                      << "  | traffic: " << trafficToString(road.traffic) << "\n";
        }
    }
    std::cout << "  ══════════════════════════════════════════\n";
    std::cout << "  Cities: " << cityCount() << "   Roads: " << roadCount() << "\n";
}

// ─────────────────────────────────────────────
//  Dijkstra (generic – weight function injected)
// ─────────────────────────────────────────────
RouteResult Graph::dijkstra_(const std::string& src,
                              const std::string& dst,
                              std::function<double(const Road&)> weightFn) const {
    RouteResult result;

    if (!cityExists(src) || !cityExists(dst)) return result;

    std::unordered_map<std::string, double> dist;
    std::unordered_map<std::string, std::string> prev;

    for (auto& kv : cities_) dist[kv.first] = INF;
    dist[src] = 0.0;

    // Min-heap: (weight, cityName)
    using P = std::pair<double, std::string>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        double w = top.first;
        std::string u = top.second;

        if (w > dist[u]) continue;
        if (u == dst)    break;

        for (const auto& road : cities_.at(u).roads) {
            double newDist = dist[u] + weightFn(road);
            if (newDist < dist[road.destination]) {
                dist[road.destination] = newDist;
                prev[road.destination] = u;
                pq.push({newDist, road.destination});
            }
        }
    }

    if (dist[dst] == INF) return result;

    // Reconstruct path
    std::vector<std::string> path;
    for (std::string cur = dst; cur != src; cur = prev[cur]) {
        path.push_back(cur);
        if (prev.find(cur) == prev.end()) return result;
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());

    // Accumulate actual stats
    result.path  = path;
    result.found = true;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const City& city = cities_.at(path[i]);
        for (const auto& road : city.roads) {
            if (road.destination == path[i + 1]) {
                result.totalDistance += road.distance;
                result.totalCost     += road.cost;
                result.totalTime     += road.distance * trafficMultiplier(road.traffic) / 60.0;
                break;
            }
        }
    }
    return result;
}

// ─────────────────────────────────────────────
//  Public Dijkstra variants
// ─────────────────────────────────────────────
RouteResult Graph::shortestRoute(const std::string& src, const std::string& dst) const {
    return dijkstra_(src, dst, [](const Road& r) { return r.distance; });
}

RouteResult Graph::cheapestRoute(const std::string& src, const std::string& dst) const {
    return dijkstra_(src, dst, [](const Road& r) { return r.cost; });
}

RouteResult Graph::fastestRoute(const std::string& src, const std::string& dst) const {
    return dijkstra_(src, dst, [](const Road& r) {
        return r.distance * trafficMultiplier(r.traffic);
    });
}

// ─────────────────────────────────────────────
//  BFS
// ─────────────────────────────────────────────
std::vector<std::string> Graph::bfsReachable(const std::string& src) const {
    std::vector<std::string> order;
    if (!cityExists(src)) return order;

    std::set<std::string>   visited;
    std::queue<std::string> bfsQueue;

    visited.insert(src);
    bfsQueue.push(src);

    while (!bfsQueue.empty()) {
        std::string cur = bfsQueue.front(); bfsQueue.pop();
        order.push_back(cur);

        std::vector<std::string> neighbours;
        for (const auto& road : cities_.at(cur).roads)
            neighbours.push_back(road.destination);
        std::sort(neighbours.begin(), neighbours.end());

        for (const auto& nb : neighbours) {
            if (!visited.count(nb)) {
                visited.insert(nb);
                bfsQueue.push(nb);
            }
        }
    }
    return order;
}

// ─────────────────────────────────────────────
//  DFS
// ─────────────────────────────────────────────
void Graph::dfsHelper_(const std::string& current,
                        std::set<std::string>& visited,
                        std::vector<std::string>& order) const {
    visited.insert(current);
    order.push_back(current);

    std::vector<std::string> neighbours;
    for (const auto& road : cities_.at(current).roads)
        neighbours.push_back(road.destination);
    std::sort(neighbours.begin(), neighbours.end());

    for (const auto& nb : neighbours) {
        if (!visited.count(nb))
            dfsHelper_(nb, visited, order);
    }
}

std::vector<std::string> Graph::dfsExplore(const std::string& src) const {
    std::vector<std::string> order;
    if (!cityExists(src)) return order;

    std::set<std::string> visited;
    dfsHelper_(src, visited, order);
    return order;
}

// ─────────────────────────────────────────────
//  File I/O
// ─────────────────────────────────────────────
bool Graph::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "CITIES\n";
    for (auto& kv : cities_)
        file << kv.first << "\n";

    file << "ROADS\n";
    std::set<std::pair<std::string,std::string>> written;
    for (auto& kv : cities_) {
        const std::string& name = kv.first;
        for (const auto& road : kv.second.roads) {
            auto key = std::make_pair(std::min(name, road.destination),
                                      std::max(name, road.destination));
            if (!written.count(key)) {
                written.insert(key);
                file << name << "|" << road.destination << "|"
                     << road.distance << "|" << road.cost << "|"
                     << trafficToString(road.traffic) << "\n";
            }
        }
    }

    file.close();
    return true;
}

bool Graph::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    cities_.clear();

    std::string line, section;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line == "CITIES" || line == "ROADS") {
            section = line;
            continue;
        }

        if (section == "CITIES") {
            addCity(line);
        } else if (section == "ROADS") {
            std::istringstream ss(line);
            std::string from, to, trafficStr;
            double dist = 0.0, cost = 0.0;

            // Parse "from|to|dist|cost|traffic"
            std::getline(ss, from, '|');
            std::getline(ss, to,   '|');
            ss >> dist; ss.ignore();
            ss >> cost; ss.ignore();
            std::getline(ss, trafficStr);

            addRoad(from, to, dist, cost, stringToTraffic(trafficStr));
        }
    }

    file.close();
    return true;
}

// ─────────────────────────────────────────────
//  BFS – actual shortest-hop PATH src → dst
//  Uses a predecessor map, exactly like Dijkstra
//  but edges are unweighted (hop count only).
// ─────────────────────────────────────────────
RouteResult Graph::bfsShortestPath(const std::string& src,
                                    const std::string& dst) const {
    RouteResult result;
    if (!cityExists(src) || !cityExists(dst)) return result;

    // prev[city] = which city we came from
    std::unordered_map<std::string, std::string> prev;
    std::set<std::string>   visited;
    std::queue<std::string> bfsQueue;

    visited.insert(src);
    bfsQueue.push(src);

    bool found = false;
    while (!bfsQueue.empty() && !found) {
        std::string cur = bfsQueue.front(); bfsQueue.pop();

        // Sort neighbours for deterministic output
        std::vector<std::string> neighbours;
        for (const auto& road : cities_.at(cur).roads)
            neighbours.push_back(road.destination);
        std::sort(neighbours.begin(), neighbours.end());

        for (const auto& nb : neighbours) {
            if (!visited.count(nb)) {
                visited.insert(nb);
                prev[nb] = cur;
                if (nb == dst) { found = true; break; }
                bfsQueue.push(nb);
            }
        }
    }

    if (!found && src != dst) return result;   // unreachable

    // Reconstruct path via predecessor map (same technique as Dijkstra)
    std::vector<std::string> path;
    for (std::string cur = dst; cur != src; cur = prev[cur]) {
        path.push_back(cur);
        if (prev.find(cur) == prev.end()) return result;  // safety guard
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());

    // Accumulate real stats along the BFS path
    result.path  = path;
    result.found = true;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        for (const auto& road : cities_.at(path[i]).roads) {
            if (road.destination == path[i + 1]) {
                result.totalDistance += road.distance;
                result.totalCost     += road.cost;
                result.totalTime     += road.distance
                                        * trafficMultiplier(road.traffic) / 60.0;
                break;
            }
        }
    }
    return result;
}
