SRCS				= 	main.c \
						srcs/error.c \
						srcs/numeric.c \
						srcs/node.c \
						srcs/parsing.c \

OBJS				= ${SRCS:.c=.o}

NAME				= ft_ping

FLAGS				= -Wall -Wextra -Werror

all :				${NAME}

${NAME} :			${OBJS}
					gcc -o ${NAME} ${FLAGS} ${SRCS}

clean :				
					rm -rf ${OBJS}

fclean : 			clean
					rm -rf ${NAME}

re :				fclean all

.PHONY:				all clean fclean re