BINARY	=	tcp_tuntap


CC	:=	cc
INCLUDE	:=	-Iinclude
CFLAGS	:=	-O3 -W -Wall -Wextra -Werror $(INCLUDE)
LDFLAGS	:=

SRC	=	src/main.c	\
		src/args.c

OBJ	=	$(SRC:.c=.o)
DEP	=	$(SRC:.c=.d)

.PHONY:	all	clean	fclean	re

all:	$(BINARY)

$(BINARY):	$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:	%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	rm -f $(OBJ) $(DEPS)

fclean:	clean
	rm -f $(BINARY)

re:	fclean	all

-include $(DEPS)
