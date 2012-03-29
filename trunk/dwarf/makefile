DWARF_LIB=/home/qali/Downloads/dwarf-20111214/libdwarf
LIBDIR= -L$(DWARF_LIB)
LIBS= -ldwarf -lelf
CFLAGS= -Wall -I$(DWARF_LIB) -O0 -g

all:
	$(CC) $(CFLAGS) parser.c -o parser $(LIBDIR) $(LIBS)
