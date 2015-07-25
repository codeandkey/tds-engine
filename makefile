CC = gcc
CFLAGS = -std=gnu99 -Wall -g
LDFLAGS = -lm -ldl -lglfw -lGL -lSOIL -lrt

SOURCES = $(wildcard src/*.c src/objects/*.c src/game/*.c)
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = tds

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(OUTPUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

cleanbin: clean
	rm -f $(OUTPUT)
