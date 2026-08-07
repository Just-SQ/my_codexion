NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
LDFLAGS		= -pthread
RM			= rm -f

HEADER		= codexion.h

SRCS		= main.c parse.c init.c cleanup.c heap.c start.c coder.c monitor.c

OBJS		= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
