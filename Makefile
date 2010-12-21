
include config.mk

SRC = mail-query.c
OBJ = ${SRC:.c=.o}

all: mail-query

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk
mail-query: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}


clean:
	rm -f mail-query ${OBJ}

.PHONY: all clean
