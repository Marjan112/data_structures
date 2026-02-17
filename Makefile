all: ll ht

ht: ht.c
	gcc -o ht ht.c -Wall -Wextra

ll: ll.c
	gcc -o ll ll.c -Wall -Wextra
