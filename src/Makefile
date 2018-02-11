all: libavdisplay.a

LIBOBJS = display_list.o display.o display_text.o

%.o : %.c %.h
	gcc -c $< -o $@

libavdisplay.a : ${LIBOBJS}
	ar rcs $@ $^

clean:
	rm -f *.a *.o *~

