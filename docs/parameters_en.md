# KCP Tube Parameters

|  Name   | Value  | Require |Note|
|  ----  | ----  | :----: | ---- |
| listen_port | 1 - 65535 |Yes|Port ranges can be specified when running as a server mode|
| destination_port | 1 - 65535 |Yes|Port ranges can be specified when running as a client mode|
| destination_address  | IP address, domain name |Yes|Brackets are not required when filling in an IPv6 address|
| dport_refresh  | 20 - 65535 |No|The unit is ‘second’. Not writting this option means using the default value of 60 seconds. <br>1 to 20 is treated as 20 seconds; greater than 32767 is treated as 32767 seconds. <br>Set to 0 means disable this option.|
| encryption_algorithm | AES-GCM<br>AES-OCB<br>chacha20<br>xchacha20<br>none |No    |AES-256-GCM-AEAD<br>AES-256-OCB-AEAD<br>ChaCha20-Poly1305<br>XChaCha20-Poly1305<br>No Encryption |
| encryption_password  | Any character |Depends…|…on the setting of encryption_algorithm, if the value is set and it is not none, it is required|
| udp_timeout  | 0 - 65535 |No|The unit is ‘second’. The default value is 1800 seconds, set to 0 to use the default value<br>This option represents the timeout setting between UDP application ↔ kcptube|
| keep_alive  | 0 - 65535 |No | The default value is 0, which means that Keep Alive is disabled. This option refers to Keep Alive between two KCP endpoints.|
| stun_server  | STUN Server's address |No| Cannot be used if listen_port option is port range mode|
| log_path  | The directory where the Logs are stored |No|Cannot point to the file itself|
| kcp_mtu  | Positive Integer |No|Default value is 1440|
| kcp  | manual<br>fast1 - 4<br>regular1 - 4<br> &nbsp; |Yes|Setup Manually<br>Fast Modes<br>Regular Speeds<br>(the number at the end: the larger the value, the slower the speed)|
| kcp_sndwnd  | Positive Integer |No|See the table below for default values, which can be overridden individually|
| kcp_rcvwnd  | Positive Integer |No|See the table below for default values, which can be overridden individually|
| kcp_nodelay  | Positive Integer |Depends…|…on the setting of ‘kcp=’, if if the value is set as ‘kcp==manual’, this option is required. See the table below for default values.|
| kcp_interval  | Positive Integer |Depends…|…on the setting of ‘kcp=’, if if the value is set as ‘kcp==manual’, this option is required. See the table below for default values.|
| kcp_resend  | Positive Integer |Depends…|…on the setting of ‘kcp=’, if if the value is set as ‘kcp==manual’, this option is required. See the table below for default values.|
| kcp_nc  | yes<br>true<br>1<br>no<br>false<br>0 |Depends…|…on the setting of ‘kcp=’, if if the value is set as ‘kcp==manual’, this option is required. See the table below for default values.|
| outbound_bandwidth | Positive Integer |No|Outbound bandwidth, used to dynamically update the value of kcp_sndwnd during communication|
| inbound_bandwidth | Positive Integer |No|Inbound bandwidth, used to dynamically update the value of kcp_rcvwnd during communication|
| ipv4_only | yes<br>true<br>1<br>no<br>false<br>0 |No|If the system disables IPv6, this option must be enabled and set to yes or true or 1|
| [listener] | N/A |Yes<br>(Relay Mode only)|Section Name of Relay Mode, KCP settings for specifying the listening mode<br>This tag represents data exchanged with the client|
| [forwarder] | N/A  |Yes<br>(Relay Mode only)|Section Name of Relay Mode, KCP settings for specifying the forwarding mode<br>This tag represents data exchanged with the server|

## outbound_bandwidth and inbound_bandwidth
Available suffixes: K / M / G

The suffix is case-sensitive, uppercase is calculated as binary (1024), lowercase is calculated as decimal (1000).

- Entering 1000 represents a bandwidth of 1000 bps

- Entering 100k represents a bandwidth of 100 kbps (100000 bps)

- Entering 100K represents a bandwidth of 100 Kbps (102400 bps)

- Entering 100M represents a bandwidth of 100 Mbps (102400 Kbps)

- Entering 1G represents a bandwidth of 1 Gbps (1024 Mbps)

Please note that it is bps (Bits Per Second), not Bps (Bytes Per Second).

## KCP Mode Default Values
| Fast Mode    | kcp_sndwnd | kcp_rcvwnd|kcp_nodelay|kcp_interval|kcp_resend|kcp_nc |
|  ----        | :----:     | :----:    | :----:    | :----:     | :----:   | ---- |
| fast1        | 2048       |   2048    |      1    |   1        |   2      |Yes|
| fast2        | 2048       |   2048    |      1    |   1        |   3      |Yes|
| fast3        | 2048       |   2048    |      1    |   5        |   2      |Yes|
| fast4        | 2048       |   2048    |      1    |   5        |   3      |Yes|

| Regular Mode | kcp_sndwnd | kcp_rcvwnd|kcp_nodelay|kcp_interval|kcp_resend|kcp_nc |
|  ----        | :----:     | :----:    | :----:    | :----:     | :----:   | ---- |
| regular1     | 1024       |   1024    |      1    |   10       |   2      |Yes|
| regular2     | 1024       |   1024    |      1    |   10       |   3      |Yes|
| regular3     | 1024       |   1024    |      0    |   10       |   2      |Yes|
| regular4     | 1024       |   1024    |      0    |   10       |   3      |Yes|

# Log File
After obtaining the IP address and port after NAT hole punching for the first time, and after the IP address and port of NAT hole punching change, an ip_address.txt file will be created in the Log directory (overwrite if it exists), and the IP address and port will be written in.

The obtained NAT hole punching address will be displayed on the console at the same time.

# STUN Servers
The STUN servers obtained from [NatTypeTeste](https://github.com/HMBSbige/NatTypeTester):
- stun.syncthing.net
- stun.qq.com
- stun.miwifi.com
- stun.bige0.com
- stun.stunprotocol.org

The STUN servers obtained from [Natter](https://github.com/MikeWang000000/Natter):

- fwa.lifesizecloud.com
- stun.isp.net.au
- stun.freeswitch.org
- stun.voip.blackberry.com
- stun.nextcloud.com
- stun.stunprotocol.org
- stun.sipnet.com
- stun.radiojar.com
- stun.sonetel.com
- stun.voipgate.com

Other STUN Servers: [public-stun-list.txt](https://gist.github.com/mondain/b0ec1cf5f60ae726202e)