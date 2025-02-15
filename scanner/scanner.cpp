#include "scanner.h"

void Scanner::updateDictionary(const std::string& ip, int port) {
    /**
     * @brief Thread safe way to update the vector of open ports for the provided target
     *
     * Uses a unique mutex lock to ensure no other thread is modifying the open ports vector.
     *
     * @param[in] ip The target that you want to add the open port too
     * @param[in] port The open port you want to add to the scanResults dictionary
     */
    std::unique_lock<std::mutex> lock(mutexMap[ip]);
    scanResults[ip].push_back(port);
}

void Scanner::loadTargets() {
    /*
    * @brief Loads the `-t` or `--target` parameter to `targets`
    * 
    * Iterates through the value to the `--target` paramter splitting by `,` and 
    * adding each target to the targets vector. Additionaly, it creates the mutex
    * and scanResults dictionary. 
    *
    */
    std::stringstream ss(targetString);
    std::string ip;
    while (std::getline(ss, ip, ',')) {
        try {
            auto address = boost::asio::ip::address::from_string(ip);
            targets.push_back(address);

            scanResults[ip];
            mutexMap[ip]; 
        }
        catch (const boost::system::system_error& e) {
            std::cerr << "Invalid IP address: " << ip << " (" << e.what() << ")\n";
        }
    }
}


void Scanner::isOpen(boost::asio::ip::address target, int port) {
    /*
    * @brief Checks to see if the provided port is open on the target IP
    * 
    * This is a async function, and utilizes an automatic timeout, to determine 
    * if a port is closed. This function does not return anything, but if the port
    * is open it will call `updateDictionary()` to push the open port to the results mapping
    * 
    * @param[in] target Boost asio IPV4 Address
    * @param[in] port   The port to connect to
    */
    semaphore.acquire();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ctx);
    auto timer = std::make_shared<boost::asio::steady_timer>(ctx);
    boost::asio::ip::tcp::endpoint endpoint(target, port);

    timer->expires_after(std::chrono::seconds(timeout));

    socket->async_connect(endpoint, [this, socket, timer, target, port](const boost::system::error_code& ec) {
        // Make sure we dont hit the connection cap
        timer->cancel();
        if (!ec && socket->is_open()) {
            updateDictionary(target.to_string(), port);
            {
                std::lock_guard<std::mutex> lock(portsMutex);
                openPorts.push_back(port);
            }
            socket->close();
        }
        semaphore.release();
        });

    timer->async_wait([socket](const boost::system::error_code& ec) {
        if (!ec)
            socket->close();
    });
}

void Scanner::displayResults() {
    /*
    @brief Displays the open ports on the scanned targets
    *
    * Iterates through `scanResults` and displays the open ports 
    * for each loaded target
    */
    for (const auto& [ip, ports] : scanResults) {
        std::cout << "Target: " << ip << "\n";
        if (ports.empty()) {
            std::cout << "No open ports found.\n\n";
        }
        else {
            std::cout << "Open Ports: \n";
            for (int port : ports) {
                std::cout << port << "/tcp \n";
            }
            std::cout << "\n" << std::endl;
        }
    }
}

std::chrono::duration<double> Scanner::getElapsed() {
    /*
    * @brief Calculates the amount of time the program has been running for
    * @return The time represented in seconds
    */
    auto end = std::chrono::high_resolution_clock::now();
    return end - startTime;
}

void Scanner::scan() {
    /*
    * @brief Scans the loaded targets, the results will be loaded in `scanResults`.
    * 
    * The asynchronous `isOpen()` function is gathered with the hardware_concurrency 
    * (minus one) threads.
    * 
    */
    for (const auto& target : targets) {
        for (int port = startPort; port <= endPort; port++) {
            isOpen(target, port);
        }
    }

    unsigned int hwThreads = std::thread::hardware_concurrency();

    // fallback and -1 sto the hinted thread count
    unsigned int threadCount = (hwThreads > 1 ? hwThreads - 1 : 1);

    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < threadCount; i++) {
        threads.emplace_back([this]() {
            ctx.run();
        });
    }

    for (auto& thread : threads)
        thread.join();
}

void Scanner::start() {
    /*
    @brief This loads the provided arguments, scans the targets, and displays the results
    */
    loadTargets();
    Scanner::scan();
    displayResults();
    double elapsedTime = getElapsed().count();
    std::cout << "Scan took " << elapsedTime << " seconds.\n";
}
