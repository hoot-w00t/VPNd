cmake_minimum_required(VERSION 3.10)

project(VPNd)

execute_process(COMMAND git describe --always --tags --abbrev=10 --dirty OUTPUT_VARIABLE GITVER OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-DGITVER="git-${GITVER}")

add_executable(vpnd
               src/vpnd.c
               src/args.c
               src/tcp4.c
               src/tcp6.c
               src/tcp.c
               src/interface.c
               src/peer.c
               src/peer_net.c
               src/protocol.c
               src/signals.c
               src/packet_header.c
               src/netroute.c
               src/scripts.c
               src/logger.c
               src/rsa.c
               src/encryption.c
               src/config.c)

target_link_libraries(vpnd pthread ssl crypto)
include_directories(include)
