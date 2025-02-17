#include "scanner.h"
#include "fingerprint.h"


std::string state_to_string(PortState state) {
    /*
    * @brief Fetches the string version of the provided port state 

    * @param[in] The port state object you want the pretty string for
    * @return a string represeting the port e.g: PortState::Open -> "OPEN"
    */
    switch (state) {
    case PortState::Open:     return "OPEN";
    case PortState::Closed:   return "CLOSED";
    case PortState::Filtered: return "FILTERED";
    default:                  return "UNKNOWN";
    }
}


boost::optional<boost::asio::ip::address> Scanner::resolveDomainFromString(const std::string& domain) {
    /*
    * @brief Takes the provided string, and attempts to DNS resolve it to a IPV4 address

    * @param[in] The domain you want to fetch the IP address for
    * @return None if the IPV4 address could not be loaded, otherwise a boost ip::address object.
    */
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::system::error_code ec;

    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoints = resolver.resolve(domain, "", ec);

    if (ec) {
        if (logger) {
            logger->debug("Failed to resolve string '{}' to due error {}", domain, ec.to_string());
        }
        return boost::none;
    }

    for (const auto& entry : endpoints) {
        boost::asio::ip::address addr = entry.endpoint().address();
        if (addr.is_v4()) {
            return addr;
        }
        logger->info("Domain '{}' resolved to an IPv6 address '{}', but IPv6 support is currently disabled. Ignoring this address.", domain, addr.to_string());
    }
    return boost::none;
}



void Scanner::updateDictionary(Target target, PortInfo portInfo) {
    /*
    * @brief A thread-safe way to update the `scanResults` vector.
    *
    * Uses a mutex to ensure no other thread is writing to the scanResults before attempting 
    * to read/write to it.
    *
    * @param[in] The target you want to update
    * @param[in] PortInfo struct holding the port and the status
    */
    std::unique_lock<std::mutex> lock(portsMutex);
    // Ensure there is an entry for the target's pretty name.
    std::vector<PortInfo>& portVec = scanResults[target.prettyName];

    bool alreadyExists = std::any_of(portVec.begin(), portVec.end(),
        [port = portInfo.port](const PortInfo& info) { return info.port == port; });

    // If we already have the port recorded for the target then return.
    if (alreadyExists) {
        logger->debug("[Scanner::updateDictionary] Port: {} on host: {} is already recorded", portInfo.port, target.prettyName);
        return;
    }

    portVec.push_back({ portInfo.port, portInfo.status });
    logger->debug("[Scanner::updateDictionary] Added port: {} to host: {} dictionary", portInfo.port, target.prettyName);
    logger->info("Discovered {} port {}/tcp on {}", state_to_string(portInfo.status), portInfo.port, target.prettyName);
}


void Scanner::loadTargets() {
    /*
    @brief loads the targets passed with the `-t` option resolving any potential domains.
    */
    std::stringstream ss(targetString);
    std::string line;
    while (std::getline(ss, line, ',')) {
        try {
            boost::asio::ip::address address = boost::asio::ip::address::from_string(line);
            Target target(address);
            targets.push_back(target);
            scanResults[target.prettyName];
        }
        catch (const boost::system::system_error& e) {
            boost::optional<boost::asio::ip::address> resolvedAddress = resolveDomainFromString(line);
            if (resolvedAddress) {
                // Use the original domain name as the pretty name.
                Target target(*resolvedAddress, line);
                targets.push_back(target);
                scanResults[target.prettyName];
            }
            else {
                if (logger) {
                    logger->error("Invalid IPV4 address or invalid domain: '{}' ({})", line, e.what());
                }
            }
        }
    }
}


void Scanner::handleSocketError(
    boost::asio::strand<boost::asio::io_context::executor_type> strand,
    boost::system::error_code ec, int port,
    Target target, int retries)
{
    /**
    * @brief Handles socket errors during asynchronous connection attempts.
    *
    * Evaluates the error code returned from a socket operation. For resource exhaustion errors
    * (boost::system::errc::resource_unavailable_try_again) with remaining retries, it schedules a
    * retry after a delay. For other errors, it either updates the port status (filtered or closed)
    * or logs the error message.
    *
    * @param strand The Boost.Asio strand executor used for posting retry operations.
    * @param ec The error code returned from the socket operation.
    * @param port The port number being scanned.
    * @param target The target being scanned.
    * @param retries The remaining number of retry attempts.
    */

    if (ec.value() == boost::system::errc::resource_unavailable_try_again && retries > 0) {
        const int& sleepTime = ((5 - retries) * 2);
        if (logger) {
            logger->debug("[Scanner::isOpen] Resource exhaustion on {}:{} - retrying in {} seconds ({} retries left)",
                target.prettyName, port, sleepTime, retries);
        }
        auto retryTimer = std::make_shared<boost::asio::steady_timer>(ctx, std::chrono::seconds(sleepTime));
        retryTimer->async_wait(boost::asio::bind_executor(strand,
            [this, target, port, retries, retryTimer](const boost::system::error_code& ecRetry) {
                if (!ecRetry) {
                    isOpen(target, port, retries - 1);
                }
            }
        ));
    }
    else if (ec == boost::asio::error::no_permission || ec.value() == 10013) {
        PortInfo portInfo = PortInfo(port, PortState::Filtered);
        updateDictionary(target, portInfo);
    }
    else if (ec == boost::asio::error::connection_refused) {
        logger->debug("In closed port branch: displayClosedPorts = {}, ec.value() = {}", displayClosedPorts, ec.value());
        if (displayClosedPorts) {
            PortInfo portInfo = PortInfo(port, PortState::Closed);
            updateDictionary(target, portInfo);
        }
    }

    else if (logger && ec != boost::asio::error::connection_refused) {
        logger->debug("[Scanner::isOpen] Connection to {}:{} failed with error: {}",
            target.prettyName, port, ec.message());
    }
}


void Scanner::throttleConnectionIfNeeded(Target target, int port, int retries) {
    /**
     * @brief Throttles connection attempts if active connections exceed the allowed maximum.
     *
     * Checks the current number of active connections. If this exceeds the maximum allowed,
     * it schedules a delayed connection attempt using a timer to avoid overloading the system.
     *
     * @param target The target host to connect to.
     * @param port The port number being scanned.
     * @param retries The remaining number of retry attempts.
     */
    if (activeConnections.fetch_add(1) >= maxConnections) {
        activeConnections.fetch_sub(1);
        if (logger) {
            logger->debug("[Scanner::isOpen] Throttling connection to {}:{} (activeConnections: {})",
                target.prettyName, port, activeConnections.load());
        }
        // add 1 to ensure the old connections have expired
        auto delayTimer = std::make_shared<boost::asio::steady_timer>(ctx, std::chrono::seconds(timeout + 1));
        delayTimer->async_wait([this, target, port, retries, delayTimer](const boost::system::error_code& ec) {
            if (!ec) {
                isOpen(target, port, retries);
            }
            });
        return;
    }
}

void Scanner::isOpen(Target target, int port, int retries) {
    /**
     * @brief Attempts to determine if a given port on a target is open.
     *
     * Applies connection throttling as necessary before asynchronously attempting a TCP
     * connection to the specified port. Uses a timeout mechanism and handles the result by
     * updating the scan results or invoking error handling.
     *
     * @param target The target host to scan.
     * @param port The port number to test.
     * @param retries The allowed number of retry attempts if connection fails.
     */
    throttleConnectionIfNeeded(target, port, retries);

    boost::asio::strand strand = boost::asio::make_strand(ctx);
    auto completed = std::make_shared<std::atomic_bool>(false);
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ctx);
    auto timer = std::make_shared<boost::asio::steady_timer>(ctx);
    boost::asio::ip::tcp::endpoint endpoint(target.address, port);
    timer->expires_after(std::chrono::seconds(timeout));

    socket->async_connect(endpoint, boost::asio::bind_executor(strand,
        [this, target, port, socket, timer, retries, strand, completed](const boost::system::error_code& ec) {
            if (!completed->exchange(true)) {
                activeConnections.fetch_sub(1);
            }
            boost::system::error_code ignore;
            timer->cancel(ignore);
            bool isPortOpen = socket->is_open();
            socket->close();
            if (!ec && isPortOpen) {
                PortInfo portInfo = PortInfo(port, PortState::Open);
                updateDictionary(target, portInfo);
            }
            else {
                handleSocketError(strand, ec, port, target, retries);
            }
        }
    ));
}


void Scanner::displayResults() {
    /**
     * @brief Displays the scan results for all targets to the console.
     *
     * Iterates through the stored scan results and prints a formatted report that includes
     * the port number, its state, and a guessed service name.
     */
    for (const auto& [targetName, ports] : scanResults) {
        std::cout << "BPS scan report for " << targetName << "\n";
        if (ports.empty()) {
            std::cout << "No open ports found.\n\n";
            continue;
        }
        std::vector<PortInfo> sortedPorts = ports;
        std::sort(sortedPorts.begin(), sortedPorts.end(), [](const PortInfo& a, const PortInfo& b) {
            return a.port < b.port;
            });

        std::cout << std::left
            << std::setw(10) << "PORT"
            << std::setw(12) << "STATE"
            << std::setw(30) << "SERVICE GUESS"
            << "\n";

        for (const auto& info : sortedPorts) {
            std::string portStr = std::to_string(info.port) + "/tcp";
            std::cout << std::left
                << std::setw(10) << portStr
                << std::setw(12) << state_to_string(info.status)
                << std::setw(26) << getServiceNameForPort(info.port)
                << "\n";
        }
        std::cout << "\n";
    }
}


float Scanner::getElapsed() const {
    /**
     * @brief Calculates the elapsed time since the scan started.
     *
     * Uses a high-resolution clock to determine the elapsed duration and rounds the result
     * to two decimal places.
     *
     * @return The elapsed time in seconds.
     */
    auto end = std::chrono::high_resolution_clock::now();
    float elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(end - startTime).count();
    return std::round(elapsed * 100.0f) / 100.0f;
}


void Scanner::scan() {
    /**
     * @brief Initiates the scanning process.
     *
     * Sets up the asynchronous I/O context and spawns a pool of threads to run the scanning tasks.
     * Posts connection tasks for each target and port in the specified range, manages the work guard,
     * and waits for all threads to finish before proceeding.
     */
    auto workGuard = boost::asio::make_work_guard(ctx);
    // gets thread hint divded by 2 with a mininum value of 1
    unsigned int threadCount = std::max(2u, std::thread::hardware_concurrency()) / 2;
    if (logger) {
        logger->debug("[Scanner::scan] Using {} threads to run the ctx context", threadCount);
    }
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < threadCount; ++i) {
        threads.emplace_back([this]() { ctx.run(); });
    }
    for (Target& target : targets) {
        for (int port = startPort; port <= endPort; ++port) {
            ctx.post([this, target, port]() {
                isOpen(target, port, 3);
                });
        }
    }
    workGuard.reset();
    if (logger) {
        logger->debug("[Scanner::scan] workGuard has been released");
    }
    int i = 1;
    for (std::thread& thread : threads) {
        thread.join();
        if (logger) {
            logger->debug("[Scanner::scan] thread {} has joined", i);
        }
        i++;
    }
}

void Scanner::createLogger() {
    /**
     * @brief Creates and configures the logger instance.
     *
     * Retrieves an existing logger or creates a new one if necessary. Sets the logging level and
     * formatting based on `isDebugMode` and `isVerboseMode`
     */
    logger = spdlog::get("bps");
    if (!logger) {
        logger = spdlog::stdout_color_mt("shared_logger");
    }
    if (isVerboseMode) {
        logger->set_level(spdlog::level::info);
    }
    else if (isDebugMode) {
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->debug("[Scanner::createLogger] Debug logging enabled.");
    }
    else {
        logger->set_level(spdlog::level::warn);
    }
}

void Scanner::loadTimingTemplate() {
    /**
     * @brief Loads the timing template to configure scanning performance parameters.
     *
     * Sets the maximum allowed connections and the connection timeout based on a timing template.
     * Logs warnings if a high-performance (and potentially error-prone) timing template is selected.
     */
    switch (timing)
    {
    // Snail speed..... (5 years later...)
    case 0:
        logger->warn("Using `-T 0` this will be extremely slow!!!!");
        maxConnections = 5;
        timeout = 8;
        break;
    case 1:
        maxConnections = 100;
        timeout = 7;
        break;
    case 2:
        maxConnections = 1000;
        timeout = 6;
        break;

    // Consistent, and fast results.
    case 3:
        maxConnections = 2500;
        timeout = 5;
        break;
    case 4:
        maxConnections = 5000;
        timeout = 4;
        break;
    case 5:
        maxConnections = 6000;
        timeout = 3;
        break;
    // Racecar, probably will display False positives
    case 6:
        maxConnections = 8000;
        timeout = 2;
        break;
    }

    if (timing >= 5 && logger)
        logger->warn("Using this high of a timing template may cause false postives");
 
    if (logger) {
        logger->debug("[Scanner::loadTimingTemplate()] Using {} max connections", maxConnections);
        logger->debug("[Scanner::loadTimingTemplate()] Connection timeout is {} seconds", timeout);
    }

}


void Scanner::start() {
    /**
     * @brief Starts the scanning operation.
     *
     * Begins the scanning process, shows the scan results,
     * and outputs the total elapsed time for the scan.
     */
    std::cout << "starting BPS (https://github.com/Drew-Alleman/bps)" << std::endl;
    scan();
    displayResults();
    float elapsedTime = getElapsed();
    //std::cout << "BPS done: " << targets.size() << " IP address scanned in "
    //    << std::fixed << std::setprecision(2) << elapsedTime << " seconds" << std::endl;
}
