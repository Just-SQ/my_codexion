NAME		:= codexion

CC			:= cc
CFLAGS		:= -Wall -Wextra -Werror
CPPFLAGS	:= -Iincludes
LDFLAGS		:= -pthread

SRC_DIR		:= src
OBJ_DIR		:= obj

SRCS		:= $(wildcard $(SRC_DIR)/*.c)
OBJS		:= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS		:= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
