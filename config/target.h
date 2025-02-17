#pragma once
#include <iostream>
#include <boost/asio.hpp>

class Target {
public:
    std::string prettyName;
    boost::asio::ip::address address;

    Target(const boost::asio::ip::address& addr)
        : address(addr), prettyName(addr.to_string()) {
    }

    Target(const boost::asio::ip::address& addr, const std::string& pretty)
        : address(addr), prettyName(pretty) {
        prettyName = pretty + " (" + addr.to_string() + ")";
    }
};
