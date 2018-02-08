TARGETS = bouncing_balls

all: ${TARGETS}

CFLAGS = -Werror -O2
INCPATH = ../src/
LIBPATH = ../src/
LIBS =  -lavdisplay -lpthread -lm -lasound

bouncing_balls: bouncing_balls.c 
	gcc ${CFLAGS} -o $@ $^ -I ${INCPATH} -L ${LIBPATH} ${LIBS}

clean:
	rm -f *.o ${TARGETS} *~
