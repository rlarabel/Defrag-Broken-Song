all: recursive defrag

recursive: recursive.c
	gcc -g -std=gnu11 -Werror -Wall recursive.c -o recursive
defrag: defrag.c
	gcc -g -std=gnu11 -Werror -lpthread -Wall defrag.c -o defrag

