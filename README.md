# (B)oost (P)ort (S)canner
A C++ tcp IPV4 port scanner utilizing [asio boost](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html) polling for speed and efficiency. 

# Usage Examples
## Basic Usage
You can provide an IP address by utiling the `-t` or `--target` argument. 
```
$ bps.exe -t 127.0.0.1
Target: 127.0.0.1
Open Ports:
135/tcp
445/tcp
902/tcp
912/tcp
3306/tcp
5040/tcp
9100/tcp
9180/tcp
27036/tcp
27060/tcp
33060/tcp
49667/tcp
49666/tcp
49665/tcp
49668/tcp
49664/tcp
49670/tcp
49680/tcp
49692/tcp
49779/tcp
49830/tcp
49831/tcp

Scan took 5.85774 seconds.
```
## Scanning Multiple Targets
You can seperate targets with comma. 
```
$ bps.exe -t 127.0.0.1,192.168.0.1,192.168.0.132,192.168.0.121,192.168.0.5 -F
Target: 192.168.0.121
No open ports found.

Target: 192.168.0.132
Open Ports:
135/tcp
139/tcp
445/tcp
902/tcp
912/tcp

Target: 127.0.0.1
Open Ports:
135/tcp
902/tcp
445/tcp
912/tcp

Target: 192.168.0.1
Open Ports:
53/tcp
80/tcp
443/tcp

Target: 192.168.0.5
No open ports found.

Scan took 3.28129 seconds.
```

## Scanning a Custom Port Range
You can specify a starting port with the `-s` or `--start` option, and a endpoint with `-e` or `--end`.
```
$ .\bps.exe -t 127.0.0.1 -s 33060 -e 49832
Target: 127.0.0.1
Open Ports:
33060/tcp
49664/tcp
49666/tcp
49665/tcp
49667/tcp
49668/tcp
49670/tcp
49680/tcp
49779/tcp
49692/tcp
49830/tcp
49831/tcp

Scan took 2.94253 seconds.
```
## Scanning Only Registerd Ports
To only scan common ports use the `-F` or `--fast` option, this will only scan ports 1-1024.
```
$ .\bps.exe -t 127.0.0.1 -F
Scanning target: 127.0.0.1:1->1024
Scan took 2.08225 seconds.
Scanned: 127.0.0.1
135/tcp
445/tcp
902/tcp
912/tcp
```

