all: trace

LC4.o: LC4.c LC4.h
	clang -g -c LC4.c

loader.o: LC4.o loader.c loader.h
	clang -g -c LC4.o loader.c
	
trace: LC4.o loader.o trace.c
	clang -g LC4.o loader.o trace.c -o trace

clean:
	rm -rf *.o

clobber: clean
	rm -rf LC4.o loader.o trace
