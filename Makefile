CC = gcc
CCFLAGS = -Wall -g -std=c99 -DDEBUG_LEVEL=10
INCLUDEDIR = ./
LDFLAGS = -lm

BASE_OBJS=algebra.o
IMAGE_OBJS=image.o io/raw.o
IMAGE_PNG_OBJS=io/lpng.o
TELESCOPE_OBJS=telescope.o
X11_OBJS=colour.o x11/draw.o x11/wevent.o

EXAMPLES=examples/startrack examples/deformation examples/xview
TESTS=tests/test_algebra

examples: ${EXAMPLES}
tests: ${TESTS}

clean:
	rm -f *.o
	rm -f */*.o
	rm -f ${EXAMPLES}
	rm -f ${TESTS}

.SUFFIXES: .c .o

.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}

examples/startrack: ${BASE_OBJS} ${TELESCOPE_OBJS} ${IMAGE_OBJS} ${IMAGE_PNG_OBJS} examples/startrack.o
	${CC} ${LDFLAGS} -lpng ${.ALLSRC} -o ${.TARGET}

examples/deformation: ${BASE_OBJS} ${TELESCOPE_OBJS} ${IMAGE_OBJS} ${IMAGE_PNG_OBJS} examples/deformation.o
	${CC} ${LDFLAGS} -lpng ${.ALLSRC} -o ${.TARGET}

examples/xview: ${BASE_OBJS} ${IMAGE_OBJS} ${X11_OBJS} examples/xview.o
	${CC} ${LDFLAGS} -lxcb ${.ALLSRC} -o ${.TARGET}

tests/test_algebra: ${BASE_OBJS} tests/test_algebra.o
	${CC} ${LDFLAGS} ${.ALLSRC} -o ${.TARGET}



