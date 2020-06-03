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

#include "peer.h"
#include "peer_net.h"
#include "protocol.h"
#include "interface.h"
#include "netroute.h"
#include "packet_header.h"
#include "logger.h"
#include "rsa.h"
#include "encryption.h"
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>

// send data to peer, with frame header
// this can encrypt data
void send_data_to_peer(uint8_t frame_hdr_type, byte_t *data, uint32_t data_len,
    bool encrypt, peer_t *peer)
{
    uint8_t frame_hdr[FRAME_HEADER_SIZE];
    byte_t enc_data[FRAME_PAYLOAD_MAXSIZE];
    uint32_t sent = 0;
    ssize_t n = 0;

    if (peer->alive) {
        if (encrypt) {
            if (!peer->authenticated) {
                logger(LOG_INFO, "peer %s:%u: not authenticated, not sending network packet",
                                 peer->address,
                                 peer->port);
                return;
            }

            int enc_len = aes_encrypt(peer->enc_ctx,
                                      data,
                                      data_len,
                                      enc_data);

            if (enc_len < 0)
                return;

            data_len = enc_len;
            data = enc_data;
        }

        encode_frame_header(frame_hdr, frame_hdr_type, data_len);
        pthread_mutex_lock(&peer->mutex);
        while (sent < sizeof(frame_hdr)) {
            n = send(peer->s, frame_hdr + sent, sizeof(frame_hdr) - sent, MSG_NOSIGNAL);
            if (n <= 0) {
                logger(LOG_ERROR, "peer %s:%u: send error: %lu/%lu bytes sent",
                                  peer->address,
                                  peer->port,
                                  sent,
                                  sizeof(frame_hdr));
                break;
            };
            sent += n;
        }

        sent = 0;
        while (sent < data_len) {
            n = send(peer->s, data + sent, data_len - sent, MSG_NOSIGNAL);
            if (n <= 0) {
                logger(LOG_ERROR, "peer %s:%u: send error: %lu/%lu bytes sent",
                                  peer->address,
                                  peer->port,
                                  sent,
                                  data_len);
                break;
            };
            sent += n;
        }
        pthread_mutex_unlock(&peer->mutex);
    }
}

// broadcast frame with data to all peers
// if *exclude if set to a peer, broadcast to all peers except this one
void broadcast_data_to_peers(uint8_t frame_hdr_type, byte_t *data, uint32_t data_len,
    bool encrypt, peer_t *exclude)
{
    peer_t *peer = get_peer_list();
    size_t sent_to = 0;

    while (peer) {
        if (peer != exclude && peer->alive) {
            send_data_to_peer(frame_hdr_type, data, data_len, encrypt, peer);
            sent_to += 1;
        }
        peer = peer->next;
    }
}

// receive frame in *buf (assuming that *buf is FRAME_MAXSIZE bytes)
// returns the number of bytes received in total, -1 on error
int receive_frame(peer_t *peer, byte_t *buf, uint8_t *header_type, uint32_t *data_len)
{
    ssize_t n = 0;
    uint32_t total = 0;

    while (total != FRAME_HEADER_SIZE) {
        n = recv(peer->s,
                 buf + total,
                 FRAME_HEADER_SIZE - total,
                 MSG_NOSIGNAL);

        if (n <= 0) {
            return -1;
        }
        total += n;
    }

    decode_frame_header(buf, header_type, data_len);

    total = 0;
    if (*data_len > FRAME_PAYLOAD_MAXSIZE) {
        logger(LOG_ERROR, "peer %s:%u: invalid payload size: %u",
                          peer->address,
                          peer->port,
                          *data_len);

        recv(peer->s, buf, FRAME_MAXSIZE, MSG_NOSIGNAL);
        return 0;

    } else if (*data_len > 0) {
        while (total < *data_len) {
            n = recv(peer->s,
                     buf + FRAME_HEADER_SIZE + total,
                     *data_len - total,
                     MSG_NOSIGNAL);

            if (n <= 0) {
                return -1;
            } else {
                total += n;
            }
        }
    }

    return total + FRAME_HEADER_SIZE;
}

void process_netpacket(peer_t *peer, byte_t *data, uint32_t data_len)
{
    netroute_t route;
    byte_t packet_data[FRAME_PAYLOAD_MAXSIZE + AES_BLOCK_SIZE];
    peer_t *target = NULL;

    int dec_len = aes_decrypt(peer->dec_ctx,
                              data,
                              data_len,
                              packet_data);

    if (dec_len < 0)
        return;

    data_len = dec_len;

    parse_packet_addr(packet_data, &route, false);
    if (is_local_route(&route)) {
        parse_packet_addr(packet_data, &route, true);

        if (!get_peer_route(&route)) {
            char addr[INET6_ADDRSTRLEN];

            memset(addr, 0, sizeof(addr));
            get_netroute_addr(&route, addr, sizeof(addr));

            logger(LOG_DEBUG, "peer %s:%u: adding route: %s",
                              peer->address,
                              peer->port,
                              addr);

            add_netroute(&route, &peer->routes);
        }

        tuntap_write(packet_data, data_len);

    } else if ((target = get_peer_route(&route))) {
        send_data_to_peer(FRAME_HDR_NETPACKET,
                          packet_data,
                          data_len,
                          true,
                          target);
    } else {
        tuntap_write(packet_data, data_len);
        broadcast_data_to_peers(FRAME_HDR_NETPACKET,
                                packet_data,
                                data_len,
                                true,
                                peer);
    }
}

int process_frame(peer_t *peer, byte_t *buf, uint8_t header_type, uint32_t data_len)
{
    byte_t *data = buf + FRAME_HEADER_SIZE;

    switch (header_type) {
        case FRAME_HDR_AUTH:
            logger(LOG_ERROR, "peer %s:%u: received unexpected auth, ignoring",
                              peer->address,
                              peer->port);
            break;

        case FRAME_HDR_CLOSE:
            logger(LOG_DEBUG, "peer %s:%u: received close",
                              peer->address,
                              peer->port);
            return -1;

        case FRAME_HDR_RESERVED:
            logger(LOG_DEBUG, "peer %s:%u: received reserved header",
                              peer->address,
                              peer->port);
            break;

        case FRAME_HDR_NETPACKET:
            if (!peer->authenticated) {
                logger(LOG_ERROR, "peer %s:%u: cannot decode network packet: not authenticated",
                                  peer->address,
                                  peer->port);
                break;
            }
            if (data_len == 0) {
                logger(LOG_ERROR, "peer %s:%u: received empty network packet",
                                  peer->address,
                                  peer->port);
                break;
            }

            process_netpacket(peer, data, data_len);
            break;

        default:
            logger(LOG_ERROR, "peer %s:%u: invalid header type: 0x%02x",
                              peer->address,
                              peer->port,
                              *buf);
            break;
    }

    return 0;
}

// authenticate *peer
// if function returns true peer is authenticated
// otherwise we should close the connection because the peer
// cannot be authenticated
bool authenticate_peer(peer_t *peer, byte_t *buf)
{
    BIO *bp = BIO_new(BIO_s_mem());
    BUF_MEM *pp = NULL;

    if (!bp) {
        logger(LOG_CRIT, "Could not allocate memory for peer authentication");
        return false;
    }

    PEM_write_bio_RSA_PUBKEY(bp, get_daemon_pubkey());
    BIO_get_mem_ptr(bp, &pp);

    byte_t *pem_key = (byte_t *) pp->data;
    uint32_t pem_key_len = pp->length;
    send_data_to_peer(FRAME_HDR_AUTH, pem_key, pem_key_len, false, peer);

    BIO_free_all(bp);

    uint8_t header_type = 0;
    uint32_t data_len = 0;

    if (receive_frame(peer, buf, &header_type, &data_len) < 0)
        return false;

    if (header_type != FRAME_HDR_AUTH || data_len == 0)
        return false;

    peer->pubkey = load_rsa_key_from_string(buf + FRAME_HEADER_SIZE, data_len, true);
    if (!peer->pubkey) {
        logger(LOG_ERROR, "peer %s:%u: received invalid invalid public key",
                          peer->address,
                          peer->port);
        return false;
    }

    if (!is_trusted_key(peer->pubkey)) {
        logger(LOG_ERROR, "peer %s:%u: untrusted public key",
                          peer->address,
                          peer->port);
        return false;
    }

    byte_t aes_key[32];
    byte_t aes_iv[16];

    if (peer->is_client) {
        // Client receives AES key and IV from server
        uint32_t _bufsize = RSA_BUFSIZE(get_daemon_privkey());
        byte_t dec_buf[_bufsize];
        int dec_len;

        if (receive_frame(peer, buf, &header_type, &data_len) < 0)
            return false;

        if (header_type != FRAME_HDR_AUTH || data_len == 0 || data_len > _bufsize)
            return false;

        dec_len = rsa_decrypt(buf + FRAME_HEADER_SIZE, data_len, dec_buf, get_daemon_privkey());
        if (dec_len != sizeof(aes_key)) {
            logger(LOG_ERROR, "Invalid AES key size");
            return false;
        }
        memcpy(aes_key, dec_buf, sizeof(aes_key));

        if (receive_frame(peer, buf, &header_type, &data_len) < 0)
            return false;

        if (header_type != FRAME_HDR_AUTH || data_len == 0 || data_len > _bufsize)
            return false;

        dec_len = rsa_decrypt(buf + FRAME_HEADER_SIZE, data_len, dec_buf, get_daemon_privkey());
        if (dec_len != sizeof(aes_iv)) {
            logger(LOG_ERROR, "Invalid AES IV size");
            return false;
        }
        memcpy(aes_iv, dec_buf, sizeof(aes_iv));

    } else {
        // Server generates and sends AES key and IV to client
        if (RAND_priv_bytes(aes_key, sizeof(aes_key)) != 1) {
            logger(LOG_ERROR, "peer %s:%u: cannot generate AES key: %s",
                            peer->address,
                            peer->port,
                            ERR_error_string(ERR_get_error(), NULL));
            return false;
        }

        if (RAND_priv_bytes(aes_iv, sizeof(aes_iv)) != 1) {
            logger(LOG_ERROR, "peer %s:%u: cannot generate AES IV: %s",
                            peer->address,
                            peer->port,
                            ERR_error_string(ERR_get_error(), NULL));
            return false;
        }

        int enc_len = rsa_encrypt(aes_key, sizeof(aes_key), buf, peer->pubkey);
        if (enc_len <= 0)
            return false;

        send_data_to_peer(FRAME_HDR_AUTH, buf, enc_len, false, peer);

        enc_len = rsa_encrypt(aes_iv, sizeof(aes_iv), buf, peer->pubkey);
        if (enc_len <= 0)
            return false;

        send_data_to_peer(FRAME_HDR_AUTH, buf, enc_len, false, peer);
    }

    peer->enc_ctx = aes_init_ctx(aes_key, aes_iv, true);
    peer->dec_ctx = aes_init_ctx(aes_key, aes_iv, false);

    memset(aes_key, 0, sizeof(aes_key));
    memset(aes_iv, 0, sizeof(aes_iv));

    if (!peer->enc_ctx || !peer->dec_ctx)
        return false;

    peer->authenticated = true;
    return true;
}

// receive data from remote peer and write it to the tun/tap device
void *peer_receive(void *arg)
{
    peer_t *peer = (peer_t *) arg;
    byte_t *buf = malloc(sizeof(byte_t) * FRAME_MAXSIZE);

    if (!buf) {
        logger(LOG_CRIT, "Could not allocate memory for peer");
        return NULL;
    }

    uint8_t header_type = 0;
    uint32_t data_len = 0;

    if (authenticate_peer(peer, buf)) {
        while (receive_frame(peer, buf, &header_type, &data_len) >= 0) {
            if (process_frame(peer, buf, header_type, data_len) < 0)
                break;
        }
    } else {
        logger(LOG_ERROR, "peer %s:%u: authentication failed",
                          peer->address,
                          peer->port);
    }

    free(buf);
    pthread_exit(NULL);
}