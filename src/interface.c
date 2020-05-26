#include "interface.h"
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// https://github.com/torvalds/linux/blob/master/Documentation/networking/tuntap.txt

static int if_fd = -1;

// return the tun/tap device file descriptor
int tuntap_fildes(void)
{
    return if_fd;
}

// close tun/tap device
void tuntap_close(void)
{
    if (if_fd > 0) {
        close(if_fd);
        if_fd = -1;
    }
}

// open tun/tap device with name *dev
// if tap_mode is true the device will be using layer 2 (TAP)
// otherwise it will use the default layer 3 (TUN)
// returns -1 on error and the file descriptor of the device on success
// also sets the static if_fd to the file descriptor
int tuntap_open(char *dev, bool tap_mode)
{
    struct ifreq ifr;
    int fd, err;

    if (if_fd > 0) {
        fprintf(stderr, "A TUN/TAP device is already open\n");
        return -1;
    }

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        fprintf(stderr, "Could not open /dev/net/tun: %s\n", strerror(errno));
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = tap_mode ? IFF_TAP : IFF_TUN;
    if (dev)
        strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        fprintf(stderr, "Could not configure %s device %s: %s\n",
                tap_mode ? "TAP" : "TUN",
                dev,
                strerror(errno));
        close(fd);
        return -1;
    }

    strcpy(dev, ifr.ifr_name);
    if_fd = fd;
    atexit(tuntap_close);

    return fd;
}

// write to the opened tun/tap device (if_fd)
ssize_t tuntap_write(void *buf, size_t n)
{
    if (if_fd < 0) {
        return -1;
    } else {
        return write(if_fd, buf, n);
    }
}

// read from the opened tun/tap device (if_fd)
ssize_t tuntap_read(void *buf, size_t nbytes)
{
    if (if_fd < 0) {
        return -1;
    } else {
        return read(if_fd, buf, nbytes);
    }
}

// return the network address from the ip and cidr
in_addr_t get_ip_network(in_addr_t ip, uint8_t cidr)
{
    in_addr_t mask = 0;
    uint8_t cidr_limit = sizeof(in_addr_t) * 8;

    for (uint8_t i = 0; i < cidr && i < cidr_limit; ++i) {
        mask |= 1 << i;
    }
    return ip & mask;
}

// set ip address on device dev to ip_addr
void set_device_ip(const char *ip_addr, uint8_t cidr, const char *dev)
{
    struct in_addr vpn_ip, vpn_network;

    vpn_ip.s_addr = inet_addr(ip_addr);
    vpn_network.s_addr = get_ip_network(vpn_ip.s_addr, cidr);

    char link_up[18 + IFNAMSIZ];
    char addr_add[40 + IFNAMSIZ];

    snprintf(link_up, sizeof(link_up), "ip link set \"%s\" up", dev);
    snprintf(addr_add, sizeof(addr_add), "ip addr add \"%s/%u\" dev \"%s\"",
             inet_ntoa(vpn_ip),
             cidr,
             dev);

    int ret;
    if ((ret = system(link_up))) {
        fprintf(stderr, "set_device_ip failed: '%s' returned %i\n",
                        link_up,
                        ret);
        return;
    }
    if ((ret = system(addr_add))) {
        fprintf(stderr, "set_device_ip failed: '%s' returned %i\n",
                        link_up,
                        ret);
        return;
    }

    printf("Set VPN IP address to %s\n", inet_ntoa(vpn_ip));
    printf("Set VPN network to %s/%u\n", inet_ntoa(vpn_network), cidr);
}