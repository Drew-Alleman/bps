# (B)oost (P)ort (S)canner
A C++ tcp IPV4 port scanner utilizing [asio boost](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) polling for speed and efficiency. 

# Usage Examples
## Basic Usage
You can provide an IP address by utiling the `-t` or `--target` argument. By default bps scans ports 1 to 10,000.
```
$ bps.exe -t 127.0.0.1
starting BPS (https://github.com/Drew-Alleman/bps)
BPS scan report for 127.0.0.1
PORT      STATE       SERVICE GUESS
135/tcp   OPEN        MS RPC
137/tcp   FILTERED    NetBIOS Name Service
445/tcp   OPEN        SMB
902/tcp   OPEN        Unknown
912/tcp   OPEN        Unknown
3306/tcp  OPEN        MySQL
5040/tcp  OPEN        Unknown
9100/tcp  OPEN        Unknown
9180/tcp  OPEN        Unknown

BPS done: 1 IP address scanned in 10.17 seconds
```
## Scanning Multiple Targets
You can seperate targets with comma. 
```
$ bps.exe -t 127.0.0.1,192.168.0.1,192.168.0.132,192.168.0.121,192.168.0.5 -F
starting BPS (https://github.com/Drew-Alleman/bps)
BPS scan report for 192.168.0.121
PORT      STATE       SERVICE GUESS
53/tcp    FILTERED    DNS

BPS scan report for 192.168.0.132
PORT      STATE       SERVICE GUESS
135/tcp   OPEN        MS RPC
137/tcp   FILTERED    NetBIOS Name Service
139/tcp   OPEN        NetBIOS Session
445/tcp   OPEN        SMB
902/tcp   OPEN        Unknown
912/tcp   OPEN        Unknown

BPS scan report for 127.0.0.1
PORT      STATE       SERVICE GUESS
135/tcp   OPEN        MS RPC
137/tcp   FILTERED    NetBIOS Name Service
445/tcp   OPEN        SMB
902/tcp   OPEN        Unknown
912/tcp   OPEN        Unknown

BPS scan report for 192.168.0.1
PORT      STATE       SERVICE GUESS
53/tcp    FILTERED    DNS
80/tcp    OPEN        HTTP
443/tcp   OPEN        HTTPS

BPS scan report for 192.168.0.5
PORT      STATE       SERVICE GUESS
53/tcp    FILTERED    DNS

BPS done: 5 IP address scanned in 16.11 seconds
```

## Scanning a Custom Port Range and Increasing the speed
You can specify a starting port with the `-s` or `--start` option, and a endpoint with `-e` or `--end`. We can also increase the speed by using `-T`. 
```
$ bps.exe -t 127.0.0.1 -s 1 -e 49832 -T 5
[warning] Using this high of a timing template may cause false postives
starting BPS (https://github.com/Drew-Alleman/bps)
BPS scan report for 127.0.0.1
PORT      STATE       SERVICE GUESS
135/tcp   OPEN        MS RPC
137/tcp   FILTERED    NetBIOS Name Service
445/tcp   OPEN        SMB
902/tcp   OPEN        Unknown
912/tcp   OPEN        Unknown
3306/tcp  OPEN        MySQL
5040/tcp  OPEN        Unknown
9100/tcp  OPEN        Unknown
9180/tcp  OPEN        Unknown
11125/tcp OPEN        Unknown
11177/tcp OPEN        Unknown
11178/tcp OPEN        Unknown
11221/tcp OPEN        Unknown
27036/tcp OPEN        Unknown
27060/tcp OPEN        Unknown
33060/tcp OPEN        Unknown
49664/tcp OPEN        Unknown
49665/tcp OPEN        Unknown
49666/tcp OPEN        Unknown
49667/tcp OPEN        Unknown
49668/tcp OPEN        Unknown
49670/tcp OPEN        Unknown
49706/tcp OPEN        Unknown

BPS done: 1 IP address scanned in 4.85 seconds
```
## Showing Closed Ports
You can display the closed ports by using `-C`. The `-F` options is used to only scan the top 1024 ports.
```
$ bps.exe -t 127.0.0.1 -F -C
starting BPS (https://github.com/Drew-Alleman/bps)
BPS scan report for 127.0.0.1
PORT      STATE       SERVICE GUESS
1/tcp     CLOSED      tcpmux
2/tcp     CLOSED      Unknown
3/tcp     CLOSED      Unknown
4/tcp     CLOSED      Unknown
...
135/tcp   OPEN        MS RPC
136/tcp   CLOSED      Unknown
137/tcp   FILTERED    NetBIOS Name Service
...
1023/tcp  CLOSED      Unknown
1024/tcp  CLOSED      Unknown

BPS done: 1 IP address scanned in 2.37 seconds
```

