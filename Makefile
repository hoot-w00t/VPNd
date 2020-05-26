BINARY	=	vpnd
GITVER	=	$(shell git describe --always --tags --abbrev=10 --dirty)

CC	:=	cc
INCLUDE	:=	-Iinclude
CFLAGS	:=	-O3 -W -Wall -Wextra $(INCLUDE) -DGITVER=\"$(GITVER)\"
LDFLAGS	:=	-lpthread

SRC	=	src/vpnd.c			\
		src/args.c			\
		src/tcp.c			\
		src/interface.c		\
		src/connection.c	\
		src/peer.c

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
