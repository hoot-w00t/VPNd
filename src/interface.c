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
#include <pthread.h>

// https://github.com/torvalds/linux/blob/master/Documentation/networking/tuntap.txt

static int if_fd = -1;
static bool if_tap = false;
static char devname[IFNAMSIZ];
static pthread_mutex_t tuntap_mutex;

// return the tun/tap device file descriptor
int tuntap_fildes(void)
{
    return if_fd;
}

// return whether we are running in tap or tun mode
bool tuntap_tap_mode(void)
{
    return if_tap;
}

// return the tun/tap device name
const char *tuntap_devname(void)
{
    return devname;
}

// close tun/tap device
void tuntap_close(void)
{
    if (if_fd > 0) {
        printf("Closing TUN/TAP device...\n");
        close(if_fd);
        if_fd = -1;
        pthread_mutex_destroy(&tuntap_mutex);
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
    strcpy(devname, ifr.ifr_name);
    if_fd = fd;
    if_tap = tap_mode;
    pthread_mutex_init(&tuntap_mutex, NULL);
    atexit(tuntap_close);

    return fd;
}

// write to the opened tun/tap device (if_fd)
ssize_t tuntap_write(void *buf, size_t n)
{
    ssize_t m = 0;
    size_t total = 0;
    uint8_t *_buf = (uint8_t *) buf;

    if (if_fd < 0) {
        return -1;
    } else {
        pthread_mutex_lock(&tuntap_mutex);
        while (total < n) {
            m = write(if_fd, &_buf[total], n - total);
            if (m <= 0) {
                fprintf(stderr, "Could not write all the data on the tun/tap device\n");
                break;
            }
            total += m;
        }
        pthread_mutex_unlock(&tuntap_mutex);
        if (total > n) {
            fprintf(stderr, "Wrote too much data on tun/tap device: %lu/%lu bytes\n", total, n);
        }
        return total;
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