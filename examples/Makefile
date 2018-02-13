TARGETS = bouncing_balls impulse_response wrencher platonic_solids astro

all: ${TARGETS}

CFLAGS = -Werror -O2
INCPATH = ../src/
LIBPATH = ../src/
LIBS =  -lavdisplay -lpthread -lm -lasound

AVDISPLAYLIB:
	cd ../src && ${MAKE}

bouncing_balls: bouncing_balls.c AVDISPLAYLIB
	gcc ${CFLAGS} -o $@ $< -I ${INCPATH} -L ${LIBPATH} ${LIBS}

impulse_response: impulse_response.c AVDISPLAYLIB
	gcc ${CFLAGS} -o $@ $< -I ${INCPATH} -L ${LIBPATH} ${LIBS}

wrencher: wrencher.c AVDISPLAYLIB
	gcc ${CFLAGS} -o $@ $< -I ${INCPATH} -L ${LIBPATH} ${LIBS}

platonic_solids: platonic_solids.c AVDISPLAYLIB
	gcc ${CFLAGS} -o $@ $< -I ${INCPATH} -L ${LIBPATH} ${LIBS}

render_text: render_text.c AVDISPLAYLIB
	gcc ${CFLAGS} -o $@ $< -I ${INCPATH} -L ${LIBPATH} ${LIBS}

astro: astro.c AVDISPLAYLIB
	gcc ${CFLAGS} -o $@ $< -I ${INCPATH} -L ${LIBPATH} ${LIBS} -lSDL2


clean:
	rm -f *.o ${TARGETS} *~
