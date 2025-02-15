
#include <boost/program_options.hpp>
#include "scanner/scanner.h"

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    int startPort = 1;
    int endPort = 65535;
    std::string targetString;
    bool isFastMode;
    int timeout;

    po::options_description desc("bps (https://github.com/Drew-Alleman/bps)");

    desc.add_options()
        ("help,h", "Displays this help message.")
        ("target,t", po::value<std::string>(&targetString)->required(), "Specify one or more target IPv4 addresses to scan (comma-separated list).")
        ("fast,F", po::bool_switch(&isFastMode)->default_value(false), "Enable fast scan mode: only scan the top 1024 most common ports.")
        ("start,s", po::value<int>(&startPort)->default_value(1), "Set the starting port number for the scan (default: 1).")
        ("end,e", po::value<int>(&endPort)->default_value(65535), "Set the ending port number for the scan (default: 65535).")
        ("timeout,w", po::value<int>(&timeout)->default_value(3), "Define the timeout (in seconds) for each port scan attempt (default: 3 seconds).");

    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        po::notify(vm);
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << desc << "\n";
        return 1;
    }

    if (isFastMode)
        endPort = 1024;

    Scanner scanner(targetString, startPort, endPort, timeout);
    scanner.start();
}
