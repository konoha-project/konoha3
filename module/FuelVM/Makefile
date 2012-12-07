target?=Debug
CC:=gcc

CFLAGS=-Wall -I.
CFLAGS_Debug=${CFLAGS} -O0 -g3
CFLAGS_Release=${CFLAGS} -O2 -g

PROGRAM=${target}/test_codegen

.PHONY : all
all: ${PROGRAM}

.PHONY : clean
clean:
	rm -f ${target}/*.o *~ ${PROGRAM}

.PHONY : test
test:
	${PROGRAM}

OBJS=${target}/codegen.o ${target}/lowering.o \
	${target}/visitor.o ${target}/FuelVM.o ${target}/test_codegen.o

${PROGRAM}: ${OBJS}
	${CC} ${CFLAGS_${target}} -o ${PROGRAM} ${OBJS}

${target}/codegen.o: codegen.c codegen.h
	${CC} ${CFLAGS_${target}} -c $< -o $*.o

${target}/lowering.o: lowering.c codegen.h visitor.h RegAlloc.h karray.h bitmap.h
	${CC} ${CFLAGS_${target}} -c $< -o $*.o

${target}/visitor.o: visitor.c codegen.h LIR.h
	${CC} ${CFLAGS_${target}} -c $< -o $*.o

${target}/test_codegen.o: test/test_codegen.c codegen.h
	${CC} ${CFLAGS_${target}} -c $< -o $*.o

${target}/FuelVM.o: FuelVM.c FuelVM.h codegen.h LIR.h
	${CC} ${CFLAGS_${target}} -c $< -o $*.o
