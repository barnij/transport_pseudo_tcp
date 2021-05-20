CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -D_GNU_SOURCE

NAME = transport
SRC = main.c stuff.c
DEPS = stuff.h
OBJS = main.o stuff.o

YOU :$(SRC) $(NAME)

$(NAME):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)

distclean:
	rm -f $(OBJS) $(NAME)