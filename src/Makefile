all: libavdisplay.a

LIBOBJS = display_list.o display.o display_text.o dlo.o vec2f.o

%.o : %.c %.h
	gcc -c $< -o $@

libavdisplay.a : ${LIBOBJS}
	ar rcs $@ $^

clean:
	rm -f *.a *.o *~

