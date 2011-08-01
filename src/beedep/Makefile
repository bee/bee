CC = gcc
CFLAGS = -Wall -g -pedantic
LDFLAGS =

PROJECT = dependencies

OBJECTS = main.o graph.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJECTS) $(PROJECT)
