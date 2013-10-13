LIBS= -ldwarf -lelf
CFLAGS= -Wall -I/usr/include -I. -O0 -g
CC1=g++

all: parser getfunction getSymbol
parser:
	$(CC) $(CFLAGS) parser.c -o parser  $(LIBS)

getfunction:
	$(CC) $(CFLAGS) getfunction.c -o getfunction  $(LIBS)
getSymbol: getSymbol.cpp
	$(CC1) $(CFLAGS) getSymbol.cpp tool.cpp -o getSymbol  $(LIBS)
