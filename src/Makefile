all: test

test: test.c display_list.c display.c primitives.c
	gcc -g -o $@ $^ -lpthread -lm -lasound
