# VPNd - a VPN daemon

## What is it?
VPNd is a learning project aimed at creating a VPN daemon that can establish secure tunnels between clients.

It is probably flawed, inefficient, and definitely not secure, but it serves as a great learning project.

## Goals / To-do list
*   [x] Handle multiple clients simultaneously
*   [x] Add authentication
*   [x] Encrypt network traffic
*   [ ] Compress network traffic
*   [ ] Add a configuration file
*   [ ] Route packets efficiently between clients
*   [x] Use a lightweight protocol to communicate between daemons
*   [ ] Add documentation
*   [x] Add IPv6 compatibility for network sockets (the VPN tunnel should already work with IPv6 packets)

## Building the project
You need `make` and `gcc` to build the project.

The `--vpn-ip` argument depends on the `ip` program to automatically set the IP address/route of the virtual device.

Clone the repository and navigate to it, then execute
```
make
```
this should compile the code in a binary named `vpnd`.
Execute `./vpnd -h` or `./vpnd --help` to learn how to use it.
