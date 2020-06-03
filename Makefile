##    VPNd - a VPN daemon
##    Copyright (C) 2020  akrocynova
##
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <https://www.gnu.org/licenses/>.

BINARY	=	vpnd
GITVER	=	git-$(shell git describe --always --tags --abbrev=10 --dirty)

CC	:=	cc
INCLUDE	:=	-Iinclude
CFLAGS	:=	-O3 -pipe -W -Wall -Wextra $(INCLUDE) -DGITVER=\"$(GITVER)\"
LDFLAGS	:=	-lpthread -pthread -lssl -lcrypto

SRC	=	src/vpnd.c			\
		src/args.c			\
		src/tcp4.c			\
		src/tcp6.c			\
		src/tcp.c			\
		src/interface.c		\
		src/peer.c			\
		src/peer_net.c		\
		src/protocol.c		\
		src/signals.c		\
		src/packet_header.c	\
		src/netroute.c		\
		src/scripts.c		\
		src/logger.c		\
		src/rsa.c			\
		src/encryption.c	\
		src/config.c

OBJ	=	$(SRC:.c=.o)
DEP	=	$(SRC:.c=.d)

.PHONY:	all	clean	fclean	re

all:	$(BINARY)

$(BINARY):	$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:	%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	rm -f $(OBJ) $(DEP)

fclean:	clean
	rm -f $(BINARY)

re:	fclean	all

-include $(DEP)
