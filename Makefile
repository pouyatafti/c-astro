CC = gcc
CCFLAGS = -Wall -g -std=c99 -DDEBUG_LEVEL=1
INCLUDEDIR = ./
LDFLAGS = -lm

BASE_OBJS=algebra.o
IMAGE_OBJS=image.o io/raw.o io/lpng.o
TELESCOPE_OBJS=telescope.o


examples: examples/startrack examples/deformation

clean:
	rm -f *.o
	rm -f */*.o
	rm -f examples/startrack
	rm -f examples/deformation

.SUFFIXES: .c .o

.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}

examples/startrack: ${BASE_OBJS} ${TELESCOPE_OBJS} ${IMAGE_OBJS} examples/startrack.o
	${CC} ${LDFLAGS} -lpng ${.ALLSRC} -o ${.TARGET}

examples/deformation: ${BASE_OBJS} ${TELESCOPE_OBJS} ${IMAGE_OBJS} examples/deformation.o
	${CC} ${LDFLAGS} -lpng ${.ALLSRC} -o ${.TARGET}




