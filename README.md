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
You need `make`, `gcc`, `git`, `pkg-config` and `openssl` to build the project.

### Installing dependencies
#### On Debian-based distros
```sh
sudo apt update
sudo apt install make gcc git pkg-config libssl-dev
```

#### On Arch Linux
```sh
sudo pacman --needed -S make gcc git pkgconf openssl
```

### Compiling the project using Make
```sh
git clone https://github.com/hoot-w00t/VPNd.git
cd VPNd
make
```
You'll get a `vpnd` binary.