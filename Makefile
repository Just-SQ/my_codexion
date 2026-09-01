NAME	= codexion

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -Iincludes

SRCS	= src/main.c src/parsing.c src/init.c src/start.c src/heap.c \
		  src/dongle.c src/dongle2.c src/coder.c src/monitor.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -pthread -o $(NAME)

src/%.o: src/%.c includes/codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
