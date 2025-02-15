#include <boost/asio.hpp>

#include <iostream>
#include <chrono>

using namespace boost::asio;

class Scanner {
public:
    Scanner(const std::string& ip, int startPort, int endPort)
        : clientIP(boost::asio::ip::address::from_string(ip)),
        ctx(),
        openPorts(),
        portsMutex(),
        startTime(std::chrono::high_resolution_clock::now()),
        startPort(startPort),
        endPort(endPort)
    {}

    int startPort;
    int endPort;
    boost::asio::ip::address clientIP;
    io_context ctx;
    std::vector<int> openPorts;
    std::mutex portsMutex;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    void isOpen(int port);
    void scanRange(int start, int end);
    // displays the open ports for the `clientIP`
    void displayResults();
    void start();
    std::chrono::duration<double> getElapsed();
};