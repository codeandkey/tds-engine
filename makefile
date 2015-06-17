CC = gcc
CFLAGS = -std=c99 -Wall
LDFLAGS = -lm -ldl -lglfw -lGL

SOURCES = $(wildcard src/*.c)
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
