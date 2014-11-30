# Syntax Highlighting Test File for Makefile
# Some more comments about this file

# Some Identifiers and Preproccessor stuff
!ifndef DEBUG
CFLAGS=-DDEBUG -g $(CFLAGS)
!else
CFLAGS=-Os $(CFLAGS)
!endif

# Some Targets
helloworld: helloworld.o
		cc -o $@ $<

helloworld.o: helloworld.c
		cc -c -o $@ $<

.PHONY: clean
clean:
		rm -f helloworld helloworld.o *~ core
