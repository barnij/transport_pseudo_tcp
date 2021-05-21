CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -D_GNU_SOURCE

NAME = transport
SRC = main2.c stuff.c
DEPS = stuff.h
OBJS = main2.o stuff.o

YOU :$(SRC) $(NAME)

$(NAME):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)

distclean:
	rm -f $(OBJS) $(NAME)