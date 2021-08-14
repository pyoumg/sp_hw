20191571.out: 20191571.o
	gcc -o 20191571.out 20191571.o  -lm

20191571.o: 20191571.h 20191571.c 20191571_assembly.c 20191571_linker.c
	gcc -c -Wall 20191571.c 

clean :
	rm *.out *.o
