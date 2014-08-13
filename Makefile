PROG=derp
MODS=derp.o oo.o plugin.o triple.o rule.o assertion.o retraction.o
CLIPSDIR=clips
CFLAGS=-Wall -c -std=c99 -D_POSIX_C_SOURCE -D_GNU_SOURCE -fpic -g -Iinclude -I$(CLIPSDIR) -DLINUX `pkg-config --cflags glib-2.0`
LFLAGS=-rdynamic -L$(CLIPSDIR) -lpthread -lclips -lm -ldl `pkg-config --libs glib-2.0`
CLIPSLIB=$(CLIPSDIR)/libclips.a

all: $(PROG) libplugin1.so libraptor.so

$(CLIPSLIB):
	cd $(CLIPSDIR); make

$(PROG): $(MODS) $(CLIPSLIB)
	gcc -o $(PROG) $(MODS) $(LFLAGS)

libplugin1.so: plugin1.o
	gcc -shared -g -o libplugin1.so plugin1.o

raptor.o: raptor.c
	gcc $(CFLAGS) `pkg-config raptor2 --cflags` -o $@ $<

libraptor.so: raptor.o
	gcc -shared -g -o libraptor.so raptor.o `pkg-config raptor2 --libs`


$(PROG).o: $(PROG).c
	gcc $(CFLAGS) -fvisibility=hidden -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROG) $(MODS) libplugin1.so plugin1.o libraptor.so raptor.o
#	cd $(CLIPSDIR); make clean

check: $(PROG)
	G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full ./$(PROG) 2>&1|less
