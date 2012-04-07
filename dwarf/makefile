DWARF_LIB=/home/gpu/qali/develop/dwarf-20111214/libdwarf
LIBDIR= -L$(DWARF_LIB)
LIBS= -ldwarf -lelf
CFLAGS= -Wall -I$(DWARF_LIB) -O0 -g

all: parser getfunction
parser:
	$(CC) $(CFLAGS) parser.c -o parser $(LIBDIR) $(LIBS)

getfunction:
	$(CC) $(CFLAGS) getfunction.c -o getfunction $(LIBDIR) $(LIBS)
