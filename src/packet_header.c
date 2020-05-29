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

// decode tuntap header and print information to stdout
void decode_tuntap_header(uint8_t *packet)
{
    uint16_t flags = (packet[0] << 8) | (packet[1]);
    uint16_t ether_type = (packet[2] << 8) | (packet[3]);

    printf("Flags: 0x%04x, proto: 0x%04x\n", flags, ether_type);
}

// decode IP packet and place the destination IP address in *dest (v4 and v6)
void ip_packet_destaddr(uint8_t *packet, uint16_t proto, netroute_t *dest)
{
    dest->mac = false;
    if (proto == ETH_P_IP) {
        struct iphdr *header = (struct iphdr *) packet;

        dest->ip4 = true;
        dest->addr[0] = (header->daddr >> 24) & 0xff;
        dest->addr[1] = (header->daddr >> 16) & 0xff;
        dest->addr[2] = (header->daddr >> 8) & 0xff;
        dest->addr[3] = (header->daddr) & 0xff;

    } else if (proto == ETH_P_IPV6) {
        struct ipv6hdr *header = (struct ipv6hdr *) packet;

        dest->ip4 = false;
        for (uint8_t i = 0; i < sizeof(dest->addr); ++i)
            dest->addr[i] = header->daddr.in6_u.u6_addr8[i];

    } else {
        logger(LOG_ERROR, "Not an IP packet, cannot inspect header");
    }
}

// decode ethernet packet and place the destination mac address in *dest
void eth_packet_destaddr(uint8_t *packet, netroute_t *dest)
{
    dest->ip4 = false;
    dest->mac = true;
    for (uint8_t i = 0; i < 6; ++i)
        dest->addr[i] = packet[i];
}

// decode tuntap packet and network packet header
// if valid, place the network packet's destination address in *dest
void packet_destaddr(uint8_t *packet, netroute_t *dest)
{
    uint8_t *raw_packet = &packet[4];

    if (tuntap_tap_mode()) {
        eth_packet_destaddr(raw_packet, dest);
    } else {
        uint16_t ether_type = (packet[2] << 8) | (packet[3]);

        ip_packet_destaddr(raw_packet, ether_type, dest);
    }
}

// decode IP packet and place the source IP address in *dest (v4 and v6)
void ip_packet_srcaddr(uint8_t *packet, uint16_t proto, netroute_t *dest)
{
    dest->mac = false;
    if (proto == ETH_P_IP) {
        struct iphdr *header = (struct iphdr *) packet;

        dest->ip4 = true;
        dest->addr[0] = (header->saddr >> 24) & 0xff;
        dest->addr[1] = (header->saddr >> 16) & 0xff;
        dest->addr[2] = (header->saddr >> 8) & 0xff;
        dest->addr[3] = (header->saddr) & 0xff;

    } else if (proto == ETH_P_IPV6) {
        struct ipv6hdr *header = (struct ipv6hdr *) packet;

        dest->ip4 = false;
        for (uint8_t i = 0; i < sizeof(dest->addr); ++i)
            dest->addr[i] = header->saddr.in6_u.u6_addr8[i];

    } else {
        logger(LOG_ERROR, "Not an IP packet, cannot inspect header");
    }
}

// decode ethernet packet and place the source mac address in *dest
void eth_packet_srcaddr(uint8_t *packet, netroute_t *dest)
{
    dest->ip4 = false;
    dest->mac = true;
    for (uint8_t i = 0; i < 6; ++i)
        dest->addr[i] = packet[i + 6];
}

// decode tuntap packet and network packet header
// if valid, place the network packet's source address in *dest
void packet_srcaddr(uint8_t *packet, netroute_t *dest)
{
    uint8_t *raw_packet = &packet[4];

    if (tuntap_tap_mode()) {
        eth_packet_srcaddr(raw_packet, dest);
    } else {
        uint16_t ether_type = (packet[2] << 8) | (packet[3]);

        ip_packet_srcaddr(raw_packet, ether_type, dest);
    }
}