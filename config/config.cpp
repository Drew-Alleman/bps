#include "config.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

void Config::sanitize() noexcept {
    /*
    * @brief Sanitizes the input feed from the user
    * 
    * 1. Ensures the endport doesnt go above 65355 or bellow 0
    * 2. If the starting port is greater then the endport, the endPort is adjusted to 65355
    * 3. Timing are stablized the maximum value of 6
    * 
    */
    auto logger = spdlog::stdout_color_mt("bps");
    logger->set_pattern("[%l] %v");
    logger->set_level(spdlog::level::debug);

    if (endPort > 65355) {
        logger->debug("end port value {} exceeds maximum allowed (65355); adjusting end port to 65355.", endPort);
        endPort = 65355;
    }

    if (startPort < 0) {
        logger->debug("startp ort value {} is below minimum (0); adjusting startPort to 0.", startPort);
        startPort = 0;
    }

    if (startPort > endPort) {
        logger->debug("start port value {} exceeds end port value {}. Adjusting end port to 65535.", startPort, endPort);
        endPort = 65535;
    }

    if (timing > 6) {
        logger->debug("Timing value {} exceeds maximum allowed (6); adjusting timing to 6.", timing);
        timing = 6;
    }
}

Config Config::load(int argc, char** argv) {
    /*
    * @brief This loads the arguments passed from the command line execution
    *
    * @param[in] argc The amount of arguments passed to main()
    * @param[argv] The text the user passed when executing the program
    */
    Config config;

    // Define command-line options using Boost Program Options
    po::options_description desc("bps (https://github.com/Drew-Alleman/bps) Options");
    desc.add_options()
        ("help,h", "Displays this help message.")
        ("target,t", po::value<std::string>(&config.targetString)->required(), "Specify one or more target IPv4 addresses to scan (comma-separated list).")
        ("fast,F", po::bool_switch(&config.isFastMode)->default_value(false), "Enable fast scan mode: only scan the top 1024 most common ports.")
        ("debug,d", po::bool_switch(&config.isDebugMode)->default_value(false), "Enable debug logging (dev and contributor logs)")
        ("verbose,v", po::bool_switch(&config.isVerboseMode)->default_value(false), "Enable verbose logging (provides additional information)")
        ("start,s", po::value<int>(&config.startPort)->default_value(1), "Set the starting port number for the scan (default: 1).")
        ("end,e", po::value<int>(&config.endPort)->default_value(10000), "Set the ending port number for the scan (default: 10000).")
        ("timing,T", po::value<int>(&config.timing)->default_value(3), "Set timing template from 0-6 (default is 3)")
        ("closed,C", po::bool_switch(&config.displayClosedPorts)->default_value(false), "Includes the closed ports on a target in the output.");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        // If help is requested, display options and exit
        if (vm.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }

        po::notify(vm);
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << desc << "\n";
        exit(1);
    }

    // Adjust the endPort if fast mode is enabled.
    if (config.isFastMode)
        config.endPort = 1024;

    // Sanitize the configuration values
    config.sanitize();

    return config;
}
