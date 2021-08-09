# ft_ping

recode the command line ping to work with ipv4 and ipv6

## options

* [-g sweepminsize] [-G sweepmaxsize] [-h sweepincrsize]
* [-v verbose]
* [-s packetsize]
* [-c count]
* [-a audible]
* [-q quietmode]


## linux

* use the following command to avoid permissions
sudo setcap cap_net_raw+ep ft_ping