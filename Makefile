CC=gcc
NAME=matrix-effect

ifdef OS
	TERM=__WIN
else
	TERM=__POSIX
endif

build: main.c
	${CC} -D ${TERM} main.c -o ${NAME}

install: ${NAME}
	cp ${NAME} /usr/local/bin/${NAME}

uninstall:
	rm /usr/local/bin/${NAME}

.PHONY clean:
	rm ${NAME}
