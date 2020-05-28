BINARY	=	vpnd
GITVER	=	git-$(shell git describe --always --tags --abbrev=10 --dirty)

CC	:=	cc
INCLUDE	:=	-Iinclude
CFLAGS	:=	-O3 -pipe -W -Wall -Wextra $(INCLUDE) -DGITVER=\"$(GITVER)\"
LDFLAGS	:=	-lpthread -pthread

SRC	=	src/vpnd.c			\
		src/args.c			\
		src/tcp.c			\
		src/interface.c		\
		src/peer.c			\
		src/peer_net.c		\
		src/protocol.c		\
		src/signals.c		\
		src/packet_header.c	\
		src/netroute.c

OBJ	=	$(SRC:.c=.o)
DEP	=	$(SRC:.c=.d)

.PHONY:	all	clean	fclean	re

all:	$(BINARY)

$(BINARY):	$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:	%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	rm -f $(OBJ) $(DEP)

fclean:	clean
	rm -f $(BINARY)

re:	fclean	all

-include $(DEP)
