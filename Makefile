CFLAGS = -Wall -Wextra -g -std=c11

all:
	${CC} ${CFLAGS} main.c -o main