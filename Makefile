berserk: berserk_run
	./berserk

berserk_run: ./berserk.c
	gcc -Wall -g -o berserk ./berserk.c
