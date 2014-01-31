PROG=derp
MODS=derp.o
CLIPSDIR=clips
CFLAGS=-Wall -c -std=c99 -fpic -g -Iinclude -I$(CLIPSDIR) -DLINUX `pkg-config --cflags glib-2.0`
LFLAGS=-L$(CLIPSDIR) -lpthread -lclips -lm -ldl `pkg-config --libs glib-2.0`
CLIPSLIB=$(CLIPSDIR)/libclips.a

all: $(PROG) libplugin1.so

$(CLIPSLIB):
	cd $(CLIPSDIR); make

$(PROG): $(MODS) $(CLIPSLIB)
	gcc -o $(PROG) $(MODS) $(LFLAGS)

libplugin1.so: plugin1.o
	gcc -shared -o libplugin1.so plugin1.o

%.o: %.c
	gcc $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROG) $(MODS)
	cd $(CLIPSDIR); make clean


