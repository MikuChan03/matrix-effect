posix:
	gcc -D__POSIX=1 -o matrix-effect main.c

win:
	gcc -D__WIN=1 -o matrix-effect main.c
