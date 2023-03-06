all: a.out b c

a.out: ./a.c
	gcc ./a.c
	
b.o: ./b.c
	gcc -c ./b.c

c.o: ./c.c
	gcc -c ./c.c

clean:
	rm -rf *.o b c a.out