
#include <boost/program_options.hpp>
#include "scanner/scanner.h"
#include "config/config.h"

namespace po = boost::program_options;

int main(int argc, char** argv) {

    Config config = Config::load(argc, argv);
    Scanner scanner(config);
    scanner.start();
    spdlog::shutdown();
    return 0;
}