CC = clang
CFLAGS = $(shell pkg-config --cflags glib-2.0) -I/usr/local/include
LDLIBS = $(shell pkg-config --libs glib-2.0) -lev -L/usr/local/lib

OBJECTS = request.o response.o server.o app.o

gg: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o app $(LDLIBS)

app.o: app.c
	$(CC) $(CFLAGS) -c app.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

resopnse.o: response.c
	$(CC) $(CFLAGS) -c response.c

request.o: request.c
	$(CC) $(CFLAGS) -c request.c

clean:
	rm *.o
