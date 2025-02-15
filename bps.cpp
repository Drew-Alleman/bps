
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

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help,h", "Displays this help message.")
        ("target,t", po::value<std::string>(&targetString)->required(), "Target IPV4 Address to scan")
        ("fast,F", po::bool_switch(&isFastMode)->default_value(false), "Only scans the top 1024 ports")
        ("start,s", po::value<int>(&startPort)->default_value(1), "Specify a starting port number")
        ("end,e", po::value<int>(&endPort)->default_value(65535), "Specify a ending port number")
        ("timeout,w", po::value<int>(&timeout)->default_value(3), "The amount of seconds to wait before determining a port is closed.");

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
