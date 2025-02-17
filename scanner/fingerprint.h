#include <unordered_map>
#include <iostream>
extern std::unordered_map<int, std::string> portToService;

// Fetches the assiocated service for the passed port
std::string getServiceNameForPort(int port);