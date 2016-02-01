CC = gcc
CFLAGS = -std=gnu99 -Wall -g -fPIC -DTDS_MEMORY_DEBUG -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pango-1.0 -I/usr/include/cairo
LDFLAGS = -shared -fPIC

SOURCES = $(wildcard src/*.c src/objects/*.c src/libs/*.c)
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = libtds.so

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(OUTPUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

cleanbin: clean
	rm -f $(OUTPUT)

install: $(OUTPUT)
	cp $(OUTPUT) /usr/lib
	mkdir -p /usr/include/tds
	cp src/*.h /usr/include/tds
