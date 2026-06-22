#include "SmartNavigator.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

SmartNavigator::SmartNavigator() {
    // Seed a few cities so the user can test right away
    graph_.addCity("Delhi");
    graph_.addCity("Jaipur");
    graph_.addCity("Agra");
    graph_.addCity("Lucknow");
    graph_.addCity("Mumbai");
    graph_.addCity("Pune");

    graph_.addRoad("Delhi",   "Jaipur",  268, 450, TrafficLevel::MEDIUM);
    graph_.addRoad("Delhi",   "Agra",    206, 350, TrafficLevel::HIGH);
    graph_.addRoad("Delhi",   "Lucknow", 555, 700, TrafficLevel::LOW);
    graph_.addRoad("Jaipur",  "Agra",    238, 400, TrafficLevel::LOW);
    graph_.addRoad("Agra",    "Lucknow", 363, 500, TrafficLevel::MEDIUM);
    graph_.addRoad("Mumbai",  "Pune",    149, 300, TrafficLevel::HIGH);
    graph_.addRoad("Lucknow", "Mumbai",  1390, 1800, TrafficLevel::LOW);
}

void SmartNavigator::run() {
    printBanner();

    while (true) {
        printMenu();
        int choice = getMenuChoice();

        switch (choice) {
            case 1:  handleAddCity();                break;
            case 2:  handleAddRoad();                break;
            case 3:  handleShortestRoute();          break;
            case 4:  handleCheapestRoute();          break;
            case 5:  handleFastestRoute();           break;
            case 6:  handleBFS();                    break;
            case 7:  handleDFS();                    break;
            case 8:  handleCompareBFSvsDijkstra();   break;
            case 9:  handleDisplayMap();             break;
            case 10: handleSaveMap();                break;
            case 11: handleLoadMap();                break;
            case 0:
                std::cout << "\n  Goodbye! Safe travels.\n\n";
                return;
            default:
                std::cout << "\n  [!] Invalid choice. Please enter 0-11.\n";
        }
    }
}

void SmartNavigator::printBanner() const {
    std::cout << R"(
  ╔══════════════════════════════════════════════════╗
  ║       SmartRoute Navigator  🗺                   ║
  ║       Mini Google Maps in C++                    ║
  ║       Graph + Dijkstra + BFS + DFS              ║
  ╚══════════════════════════════════════════════════╝
)";
}

void SmartNavigator::printMenu() const {
    std::cout << R"(
  ┌────────────────────────────────────────────────┐
  │                  MAIN MENU                    │
  ├────────────────────────────────────────────────┤
  │  1.  Add a new city                           │
  │  2.  Add a road between cities                │
  │  3.  Shortest route    (Dijkstra – distance)  │
  │  4.  Cheapest route    (Dijkstra – cost)      │
  │  5.  Fastest route     (Dijkstra – traffic)   │
  │  6.  All reachable cities  (BFS)              │
  │  7.  Explore map           (DFS)              │
  │  8.  Compare BFS vs Dijkstra                  │
  │  9.  Display full map                         │
  │  10. Save map to file                         │
  │  11. Load map from file                       │
  │  0.  Exit                                     │
  └────────────────────────────────────────────────┘
)";
}

void SmartNavigator::printDivider() const {
    std::cout << "\n  ──────────────────────────────────────────\n";
}

int SmartNavigator::getMenuChoice() const {
    int choice = -1;
    std::cout << "  Enter choice: ";
    std::cin >> choice;

    // Clear bad input
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}


std::string SmartNavigator::getCityInput(const std::string& prompt) const {
    std::string name;
    std::cout << "  " << prompt;
    std::getline(std::cin, name);

  
    size_t start = name.find_first_not_of(" \t");
    size_t end   = name.find_last_not_of(" \t");
    if (start == std::string::npos) return "";
    return name.substr(start, end - start + 1);
}

double SmartNavigator::getPositiveDouble(const std::string& prompt) const {
    double val = -1;
    while (val <= 0) {
        std::cout << "  " << prompt;
        std::cin >> val;
        if (std::cin.fail() || val <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  [!] Please enter a positive number.\n";
            val = -1;
        }
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return val;
}

TrafficLevel SmartNavigator::getTrafficLevel() const {
    std::cout << "  Traffic level (1=LOW, 2=MEDIUM, 3=HIGH): ";
    int t = 1;
    std::cin >> t;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (t == 3) return TrafficLevel::HIGH;
    if (t == 2) return TrafficLevel::MEDIUM;
    return TrafficLevel::LOW;
}

void SmartNavigator::printCityList() const {
    if (graph_.cityCount() == 0) {
        std::cout << "  (No cities yet)\n";
        return;
    }
    std::cout << "  Cities in map: " << graph_.cityCount() << "\n";
}

void SmartNavigator::printVisitOrder(const std::vector<std::string>& order,
                                      const std::string& label) const {
    std::cout << "\n  " << label << ":\n  ";
    for (size_t i = 0; i < order.size(); ++i) {
        std::cout << order[i];
        if (i + 1 < order.size()) std::cout << " → ";
    }
    std::cout << "\n";
}


// 1. Add City
void SmartNavigator::handleAddCity() {
    printDivider();
    std::string name = getCityInput("Enter city name: ");
    if (name.empty()) {
        std::cout << "  [!] City name cannot be empty.\n";
        return;
    }
    if (graph_.addCity(name))
        std::cout << "  [✓] City \"" << name << "\" added successfully.\n";
    else
        std::cout << "  [!] City \"" << name << "\" already exists.\n";
}

// 2. Add Road
void SmartNavigator::handleAddRoad() {
    printDivider();
    graph_.displayMap();   // show current cities so user knows what exists

    std::string from = getCityInput("Enter source city     : ");
    std::string to   = getCityInput("Enter destination city: ");

    if (!graph_.cityExists(from)) {
        std::cout << "  [!] City \"" << from << "\" not found.\n"; return;
    }
    if (!graph_.cityExists(to)) {
        std::cout << "  [!] City \"" << to   << "\" not found.\n"; return;
    }
    if (from == to) {
        std::cout << "  [!] Source and destination cannot be the same.\n"; return;
    }

    double dist = getPositiveDouble("Distance (km)         : ");
    double cost = getPositiveDouble("Travel cost (Rs.)     : ");
    TrafficLevel traffic = getTrafficLevel();

    if (graph_.addRoad(from, to, dist, cost, traffic))
        std::cout << "  [✓] Road added: " << from << " ↔ " << to << "\n";
    else
        std::cout << "  [!] Could not add road. Check city names or values.\n";
}

// 3. Shortest route (by distance)
void SmartNavigator::handleShortestRoute() {
    printDivider();
    std::cout << "  ─── SHORTEST ROUTE (Dijkstra / Distance) ───\n";
    std::string src = getCityInput("Enter source city     : ");
    std::string dst = getCityInput("Enter destination city: ");

    RouteResult result = graph_.shortestRoute(src, dst);
    result.print();
}

// 4. Cheapest route (by cost)
void SmartNavigator::handleCheapestRoute() {
    printDivider();
    std::cout << "  ─── CHEAPEST ROUTE (Dijkstra / Cost) ───\n";
    std::string src = getCityInput("Enter source city     : ");
    std::string dst = getCityInput("Enter destination city: ");

    RouteResult result = graph_.cheapestRoute(src, dst);
    result.print();
}

// 5. Fastest route (by traffic-adjusted time)
void SmartNavigator::handleFastestRoute() {
    printDivider();
    std::cout << "  ─── FASTEST ROUTE (Dijkstra / Traffic-Adjusted) ───\n";
    std::string src = getCityInput("Enter source city     : ");
    std::string dst = getCityInput("Enter destination city: ");

    RouteResult result = graph_.fastestRoute(src, dst);
    result.print();
}

// 6. BFS reachable cities
void SmartNavigator::handleBFS() {
    printDivider();
    std::cout << "  ─── BFS REACHABLE CITIES ───\n";
    std::string src = getCityInput("Enter starting city: ");

    auto order = graph_.bfsReachable(src);
    if (order.empty()) {
        std::cout << "  [!] City not found or no connections.\n";
        return;
    }
    printVisitOrder(order, "BFS traversal order");
    std::cout << "  Total reachable: " << order.size() << " cities\n";
}

// 7. DFS explore
void SmartNavigator::handleDFS() {
    printDivider();
    std::cout << "  ─── DFS MAP EXPLORATION ───\n";
    std::string src = getCityInput("Enter starting city: ");

    auto order = graph_.dfsExplore(src);
    if (order.empty()) {
        std::cout << "  [!] City not found or no connections.\n";
        return;
    }
    printVisitOrder(order, "DFS traversal order");
    std::cout << "  Total explored: " << order.size() << " cities\n";
}

// 8. Compare BFS vs Dijkstra
void SmartNavigator::handleCompareBFSvsDijkstra() {
    printDivider();
    std::cout << "  ─── BFS vs DIJKSTRA COMPARISON ───\n";
    std::string src = getCityInput("Enter source city     : ");
    std::string dst = getCityInput("Enter destination city: ");

    if (!graph_.cityExists(src)) {
        std::cout << "  [!] City \"" << src << "\" not found.\n"; return;
    }
    if (!graph_.cityExists(dst)) {
        std::cout << "  [!] City \"" << dst << "\" not found.\n"; return;
    }
    RouteResult bfsResult = graph_.bfsShortestPath(src, dst);

    std::cout << "\n  [BFS – fewest hops / unweighted]\n";
    if (!bfsResult.found) {
        std::cout << "  No path found from " << src << " to " << dst << " (BFS).\n";
    } else {
        std::cout << "  Path   : ";
        for (size_t i = 0; i < bfsResult.path.size(); ++i) {
            std::cout << bfsResult.path[i];
            if (i + 1 < bfsResult.path.size()) std::cout << "  -->  ";
        }
        std::cout << "\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Hops   : " << (bfsResult.path.size() - 1) << "  (fewest possible stops)\n";
        std::cout << "  Dist   : " << bfsResult.totalDistance << " km  (actual, not optimised)\n";
        std::cout << "  Cost   : Rs. " << bfsResult.totalCost << "  (actual, not optimised)\n";
        std::cout << "  Note   : BFS ignores weights; it finds the fewest-stop path only.\n";
    }

    // ── Dijkstra path: optimised by distance ──────────────────────
    RouteResult dijkResult = graph_.shortestRoute(src, dst);
    std::cout << "\n  [Dijkstra – shortest distance, weight-aware]\n";
    dijkResult.print();

    
    if (bfsResult.found && dijkResult.found) {
        std::cout << "\n  COMPARISON:\n";
        std::cout << "  BFS hops     : " << (bfsResult.path.size() - 1)
                  << "  |  Dijkstra hops : " << (dijkResult.path.size() - 1) << "\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  BFS distance : " << bfsResult.totalDistance
                  << " km  |  Dijkstra dist : " << dijkResult.totalDistance << " km\n";
        if (bfsResult.path == dijkResult.path)
            std::cout << "  Result: Both algorithms agree on the same path here!\n";
        else
            std::cout << "  Result: Paths DIFFER — BFS chose fewer hops, Dijkstra chose\n"
                      << "          shorter total distance. This shows why weights matter.\n";
    }

    std::cout << "\n  KEY INSIGHT:\n"
              << "  BFS = fewest stops (good for unweighted/hop problems).\n"
              << "  Dijkstra = minimum total weight (correct for maps/routing).\n";
}

// 9. Display map
void SmartNavigator::handleDisplayMap() {
    graph_.displayMap();
}

// 10. Save map to file
void SmartNavigator::handleSaveMap() {
    printDivider();
    std::string filename = getCityInput("Enter filename to save (e.g. data/my_map.txt): ");
    if (filename.empty()) filename = "data/saved_map.txt";

    if (graph_.saveToFile(filename))
        std::cout << "  [✓] Map saved to: " << filename << "\n";
    else
        std::cout << "  [!] Could not save. Check if the directory exists.\n";
}

// 11. Load map from file
void SmartNavigator::handleLoadMap() {
    printDivider();
    std::string filename = getCityInput("Enter filename to load (e.g. data/sample_map.txt): ");
    if (filename.empty()) filename = "data/sample_map.txt";

    if (graph_.loadFromFile(filename))
        std::cout << "  [✓] Map loaded from: " << filename
                  << " (" << graph_.cityCount() << " cities, "
                  << graph_.roadCount() << " roads)\n";
    else
        std::cout << "  [!] Could not load file. Check the filename.\n";
}
