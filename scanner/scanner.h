#include <boost/asio.hpp>
#include <boost/optional.hpp>

#define FMT_UNICODE 0
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "config/config.h"
#include "config/target.h"

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>    
#include <thread>

enum class PortState {
    Open,
    Closed,
    Filtered,
    Unknown
};

struct PortInfo {
    int port;
    PortState status;
};

using namespace boost::asio;

class Scanner {
public:
    Scanner(const Config& config)
        : targetString(config.targetString),
        startPort(config.startPort),
        endPort(config.endPort),
        isDebugMode(config.isDebugMode),
        isVerboseMode(config.isVerboseMode),
        timing(config.timing),
        displayClosedPorts(config.displayClosedPorts)
    {
        createLogger();
        loadTimingTemplate();
        loadTargets();
        startTime = std::chrono::high_resolution_clock::now();
    }

    std::string targetString;
    int startPort;
    int endPort;
    int timeout;
    int timing;
    bool isDebugMode;
    bool isVerboseMode;
    bool displayClosedPorts;

    std::vector<Target> targets;
    io_context ctx;

    std::unordered_map<std::string, std::mutex> mutexMap;
    std::unordered_map<std::string, std::vector<PortInfo>> scanResults;
    std::mutex portsMutex;
    std::shared_ptr<spdlog::logger> logger;

    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    // New member variables for throttling
    std::atomic<int> activeConnections;
    int maxConnections;

    // Configures the logger.
    void createLogger();
    // Configures `timeout` and `maxConnections` based on the value of `timing`
    void loadTimingTemplate();
    // Takes the provided string, and attempts to DNS resolve it to a IPV4 address
    boost::optional<boost::asio::ip::address> resolveDomainFromString(const std::string& domain);
    // Loads the IP addresses from --targets.
    void loadTargets();
    // Updates the vector of open ports for the provided target.
    void updateDictionary(Target target, PortInfo portInfo);
    // Checks to see if the provided port is open on the target IP.
    void isOpen(Target, int port, int retries);
    void handleSocketError(
        boost::asio::strand<boost::asio::io_context::executor_type> strand,
        boost::system::error_code ec, int port,
        Target target, int retries = 3);
    // Scans the loaded targets; results will be stored in `scanResults`.
    void scan();
    // If the max amount of connections are met, we simply wait here
    void throttleConnectionIfNeeded(Target, int port, int retries);
    // Loads arguments, scans the targets, and displays results.
    void start();
    // Displays the open ports on the scanned targets.
    void displayResults();
    // Calculates the elapsed time.
    float getElapsed() const;
};