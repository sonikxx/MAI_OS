FLAGS = -O2 -pthread

all: a.out

a.out: main.c
	gcc $(FLAGS) main.c -o a.out -lrt

child: child.c
	gcc $(FLAGS) child.c -o child -lrt

clean:
	rm -rf *.out
