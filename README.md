# (B)oost (P)ort (S)canner
A C++ tcp IPV4 port scanner utilizing [asio boost](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) polling for speed and efficiency. 

# Usage Examples
## Basic Usage
You can provide an IP address by utiling the `-t` or `--target` argument. 
```
PS C:\Users\Drew\Desktop\bps\x64\Release> .\bps.exe -t 127.0.0.1
Scanning target: 127.0.0.1:1->65535
Scan took 5.67628 seconds.
Scanned: 127.0.0.1
135/tcp
445/tcp
902/tcp
912/tcp
3306/tcp
```
## Scanning a Custom Port Range
You can specify a starting port with the `-s` or `--start` option, and a endpoint with `-e` or `--end`.
```
PS C:\Users\DrewQ\Desktop\bps\x64\Release> .\bps.exe -t 127.0.0.1 -s 33060 -e 49832
Scanning target: 127.0.0.1:33060->49832
Scan took 2.94457 seconds.
Scanned: 127.0.0.1
33060/tcp
49664/tcp
49665/tcp
49666/tcp
49667/tcp
49668/tcp
49670/tcp
49680/tcp
49692/tcp
49779/tcp
49830/tcp
49831/tcp
```
## Scanning Only Registerd Ports
To only scan common ports use the `-F` or `--fast` option, this will only scan ports 1-1024.
```
PS C:\Users\DrewQ\Desktop\bps\x64\Release> .\bps.exe -t 127.0.0.1 -F
Scanning target: 127.0.0.1:1->1024
Scan took 2.08225 seconds.
Scanned: 127.0.0.1
135/tcp
445/tcp
902/tcp
912/tcp
```

