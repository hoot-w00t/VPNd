/*
    VPNd - a VPN daemon
    Copyright (C) 2020  akrocynova

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "structs.h"
#include "interface.h"
#include "netroute.h"
#include "logger.h"
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <stdio.h>
#include <string.h>

// decode tuntap header and print information to stdout
void decode_tuntap_header(const byte_t *packet)
{
    uint16_t flags = (packet[0] << 8) | (packet[1]);
    uint16_t ether_type = (packet[2] << 8) | (packet[3]);

    printf("Flags: 0x%04x, proto: 0x%04x\n", flags, ether_type);
}

// decode IP packet and place the source/destination IP address
// in *nroute (v4 and v6)
void parse_ip_packet(const byte_t *packet, const uint16_t proto,
    netroute_t *nroute, const bool source)
{
    nroute->mac = false;

    if (proto == ETH_P_IP) {
        struct iphdr *header = (struct iphdr *) packet;

        nroute->ip4 = true;
        uint32_t ip4_addr;

        if (source) {
            ip4_addr = header->saddr;
        } else {
            ip4_addr = header->daddr;
        }

        nroute->addr[0] = (ip4_addr >> 24) & 0xff;
        nroute->addr[1] = (ip4_addr >> 16) & 0xff;
        nroute->addr[2] = (ip4_addr >> 8) & 0xff;
        nroute->addr[3] = (ip4_addr) & 0xff;
    } else {
        struct ipv6hdr *header = (struct ipv6hdr *) packet;

        nroute->ip4 = false;
        byte_t *ip6_addr;

        if (source) {
            ip6_addr = header->saddr.in6_u.u6_addr8;
        } else {
            ip6_addr = header->daddr.in6_u.u6_addr8;
        }

        memcpy(nroute->addr,
               ip6_addr,
               sizeof(nroute->addr));
    }
}

// decode ethernet packet and place the source/destination mac address
// in *nroute
void parse_eth_packet(const byte_t *packet, netroute_t *nroute,
    const bool source)
{
    nroute->ip4 = false;
    nroute->mac = true;

    if (source) {
        memcpy(nroute->addr, packet + 6, 6);
    } else {
        memcpy(nroute->addr, packet, 6);
    }
}

// decode tuntap packet and network packet header
// if valid, place the network packet's source/destination
// address in *nroute
void parse_packet_addr(byte_t *packet, netroute_t *nroute,
    const bool source)
{
    if (tuntap_tap_mode()) {
        parse_eth_packet(packet + 4, nroute, source);
    } else {
        uint16_t ether_type = (packet[2] << 8) | (packet[3]);

        if (ether_type != ETH_P_IP && ether_type != ETH_P_IPV6) {
            logger(LOG_ERROR, "Not an IP packet, cannot inspect header");
            return;
        }
        parse_ip_packet(packet + 4, ether_type, nroute, source);
    }
}