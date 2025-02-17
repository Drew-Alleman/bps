#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct Config {
    std::string targetString;
    int startPort;
    int endPort;
    int timing;
    bool isFastMode;
    bool isDebugMode;
    bool isVerboseMode;
    bool displayClosedPorts;

    // Sanitize the configuration values
    void sanitize() noexcept;

    // Load config from command-line arguments and sanitize
    static Config load(int argc, char** argv);
};

#endif // CONFIG_H
