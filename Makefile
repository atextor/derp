PROG=derp
MODS=derp.o
CLIPSDIR=clips
CFLAGS=-Wall -c -std=c99 -D_POSIX_C_SOURCE -fpic -g -Iinclude -I$(CLIPSDIR) -DLINUX `pkg-config --cflags glib-2.0`
LFLAGS=-rdynamic -L$(CLIPSDIR) -lpthread -lclips -lm -ldl `pkg-config --libs glib-2.0`
CLIPSLIB=$(CLIPSDIR)/libclips.a

all: $(PROG) libplugin1.so

$(CLIPSLIB):
	cd $(CLIPSDIR); make

$(PROG): $(MODS) $(CLIPSLIB)
	gcc -o $(PROG) $(MODS) $(LFLAGS)

libplugin1.so: plugin1.o
	gcc -shared -g -o libplugin1.so plugin1.o

$(PROG).o: $(PROG).c
	gcc $(CFLAGS) -fvisibility=hidden -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROG) $(MODS) libplugin1.so plugin1.o
#	cd $(CLIPSDIR); make clean


