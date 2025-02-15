#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>    
#include <thread>
#include <semaphore>

using namespace boost::asio;

class Scanner {

public:
    Scanner(const std::string& ip, int startPort, int endPort, int timeout)
        : targetString(ip),
        ctx(),
        openPorts(),
        portsMutex(),
        startTime(std::chrono::high_resolution_clock::now()),
        startPort(startPort),
        endPort(endPort),
        timeout(timeout)
    {}

    std::string targetString;
    int startPort;
    int endPort;
    int timeout;
    int actions;
    std::vector<boost::asio::ip::address> targets;
    io_context ctx;
    std::unordered_map<std::string, std::mutex> mutexMap;
    std::unordered_map<std::string, std::vector<int>> scanResults;
    std::vector<int> openPorts;
    std::mutex portsMutex;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    // Loads the IP addresses from --targets
    void loadTargets();
    // Updates the vector of open ports for the provided target
    void updateDictionary(const std::string& key, int value);
    // Checks to see if the provided port is open on the target IP
    void isOpen(boost::asio::ip::address target, int port);
    // Scans the loaded targets, the results will be loaded in `scanResults`.
    void scan();
    // This loads the provided arguments, scans the targets, and displays the results
    void start();
    // Displays the open ports on the scanned targets
    void displayResults();
    // Calculates the amount of time the program has been running for
    std::chrono::duration<double> getElapsed();

private:
    std::counting_semaphore<10000> semaphore{ 10000 };
};