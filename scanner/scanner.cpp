#include "scanner.h"

void Scanner::isOpen(int port) {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ctx);
    boost::asio::ip::tcp::endpoint endpoint(clientIP, port);
    // use async to take advantage of ctx polling for performance
    socket->async_connect(endpoint, [this, socket, port](const boost::system::error_code& ec) {
        // Just using is_open() is not accurate enough to determine the port status
        if (socket->is_open() && !ec) {
            std::lock_guard<std::mutex> lock(portsMutex);
            openPorts.push_back(port);
            socket->close();
        }
    });
}

void Scanner::scanRange(int start, int end) {
    for (int port = start; port <= end; port++) {
        isOpen(port);
    }
    ctx.run();
    std::sort(openPorts.begin(), openPorts.end());
}


void Scanner::displayResults() {
    std::cout << "Scanned: " << clientIP << std::endl;
    for (int port : openPorts) {
        std::cout << port << "/tcp\n";
    }
}

std::chrono::duration<double> Scanner::getElapsed() {
    auto end = std::chrono::high_resolution_clock::now();
    return end - startTime;
}

void Scanner::start() {
    std::cout << "Scanning target: " << clientIP << ":" << startPort << "->" << endPort << "\n";
    scanRange(startPort, endPort);
    double elapsedTime = getElapsed().count();
    std::cout << "Scan took " << elapsedTime << " seconds.\n";

}
