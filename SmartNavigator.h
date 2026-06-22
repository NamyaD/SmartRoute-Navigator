#ifndef SMART_NAVIGATOR_H
#define SMART_NAVIGATOR_H

#include "Graph.h"

// ─────────────────────────────────────────────
//  SmartNavigator
//  Wraps the Graph and owns the console UI.
//  Follows a simple MVC-like separation:
//    Graph      → Model (data + algorithms)
//    SmartNavigator → Controller + View (menu + display)
// ─────────────────────────────────────────────
class SmartNavigator {
public:
    SmartNavigator();

    // Entry point – starts the interactive menu loop
    void run();

private:
    Graph graph_;

    // ── Menu helpers ─────────────────────────
    void printBanner()   const;
    void printMenu()     const;
    void printDivider()  const;
    int  getMenuChoice() const;

    // ── Feature handlers (called from run()) ─
    void handleAddCity();
    void handleAddRoad();
    void handleShortestRoute();
    void handleCheapestRoute();
    void handleFastestRoute();
    void handleBFS();
    void handleDFS();
    void handleCompareBFSvsDijkstra();
    void handleDisplayMap();
    void handleSaveMap();
    void handleLoadMap();

    // ── Input helpers ────────────────────────
    std::string getCityInput(const std::string& prompt) const;
    double      getPositiveDouble(const std::string& prompt) const;
    TrafficLevel getTrafficLevel() const;

    // ── Display helpers ──────────────────────
    void printCityList()     const;
    void printVisitOrder(const std::vector<std::string>& order,
                         const std::string& label) const;
};

#endif // SMART_NAVIGATOR_H
