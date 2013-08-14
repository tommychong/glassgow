CC = clang
CFLAGS = $(shell pkg-config --cflags glib-2.0)
LDLIBS = $(shell pkg-config --libs glib-2.0)

debug:
	$(CC) $(CFLAGS) app.c -g -o app $(LDLIBS)

gg:
	$(CC) $(CFLAGS) app.c -o app $(LDLIBS)

clean:
	rm *.o
