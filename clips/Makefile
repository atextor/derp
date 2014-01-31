GCC=gcc
OBJS=$(patsubst %.c, %.o, $(wildcard *.c))
OBJSEXCEPTMAIN=$(filter-out main.o, $(OBJS))
CFLAGS=-Wall -Wundef -Wpointer-arith -Wshadow -Wcast-qual -Wcast-align -Winline -Wmissing-declarations -Wredundant-decls -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -Waggregate-return -Wno-implicit 
LDFLAGS=-lm -ltermcap
BIN=clips
LIB=libclips.a

$(LIB): $(OBJSEXCEPTMAIN)
	ar r $(LIB) $^

$(BIN): $(OBJS)
	$(GCC) -o $(BIN) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(GCC) -c $(CFLAGS) $<

clean:
	-rm -f $(BIN)
	-rm -f $(OBJS)
	-rm -f $(LIB)

