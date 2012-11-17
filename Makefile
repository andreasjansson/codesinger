LIBS = -lsndfile
CFLAGS = -Wall -g
LDFLAGS = ${LIBS}
CC = gcc

all: codesinger

codesinger: codesinger.c
	${CC} -o codesinger ${CFLAGS} ${LIBS} codesinger.c
