#include "fingerprint.h"

#include <unordered_map>
#include <string>

std::string getServiceNameForPort(int port) {
    /*
    * @brief Fetches the associated service name for the provided port
    * 
    * @param[in] the port value to check
    * @return the pretty name of the service that is typically associated for that port
    */
    auto it = portToService.find(port);
    return (it != portToService.end()) ? it->second : "Unknown";
}

std::unordered_map<int, std::string> portToService {
    {   1, "tcpmux" },
    {   7, "Echo" },
    {   9, "Discard" },
    {  13, "Daytime" },
    {  17, "Quote of the Day" },
    {  19, "CHARGEN" },
    {  20, "FTP Data" },
    {  21, "FTP Control" },
    {  22, "SSH" },
    {  23, "Telnet" },
    {  25, "SMTP" },
    {  37, "Time" },
    {  42, "Nameserver" },
    {  43, "WHOIS" },
    {  49, "TACACS" },
    {  53, "DNS" },
    {  67, "DHCP Server" },
    {  68, "DHCP Client" },
    {  69, "TFTP" },
    {  70, "Gopher" },
    {  79, "Finger" },
    {  80, "HTTP" },
    {  88, "Kerberos" },
    { 110, "POP3" },
    { 111, "RPCbind" },
    { 113, "Ident" },
    { 119, "NNTP" },
    { 123, "NTP" },
    { 135, "MS RPC" },
    { 137, "NetBIOS Name Service" },
    { 138, "NetBIOS Datagram" },
    { 139, "NetBIOS Session" },
    { 143, "IMAP" },
    { 161, "SNMP" },
    { 162, "SNMP Trap" },
    { 179, "BGP" },
    { 389, "LDAP" },
    { 443, "HTTPS" },
    { 445, "SMB" },
    { 465, "SMTPS" },
    { 587, "SMTP (Submission)" },
    { 631, "IPP" },
    { 636, "LDAPS" },
    { 993, "IMAP (SSL)" },
    { 995, "POP3 (SSL)" },
    {1025, "NFS / IIS" },
    {1080, "SOCKS Proxy" },
    {1194, "OpenVPN" },
    {1433, "MSSQL" },
    {1521, "Oracle DB" },
    {1723, "PPTP" },
    {1900, "SSDP" },
    {2049, "NFS" },
    {2121, "FTP Alternative" },
    {3128, "Squid Proxy" },
    {3306, "MySQL" },
    {3389, "RDP" },
    {4000, "ICQ" },
    {4443, "Alternate HTTPS" },
    {4500, "IPsec NAT-Traversal" },
    {5000, "UPnP" },
    {5001, "UPnP" },
    {5432, "PostgreSQL" },
    {5353, "mDNS" },
    {5500, "VNC Alt" },
    {5800, "VNC over HTTP" },
    {5900, "VNC" },
    {6000, "X11" },
    {8000, "HTTP Alt" },
    {8080, "HTTP-Alt" },
    {8443, "HTTPS Alt" },
    {8888, "Alternate HTTP" },
    {9000, "HTTP Alt" },
    {9090, "Web Management" },
    {10000, "Webmin" },
    {11211, "Memcached" },
    {27017, "MongoDB" },
    {32400, "Plex"},
    {28017, "MongoDB HTTP" }
};