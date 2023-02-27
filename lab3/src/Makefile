CC = gcc
LD = gcc
CFLAGS = -pthread
LDFLAGS =#

SRC = main.c
OBJ=$(SRC:.c=.o)
EXE=a.out

all: $(EXE)
	
$(EXE): $(OBJ)
	$(LD) $(LDFLAGS) -o $(EXE) $(OBJ)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o a.out
