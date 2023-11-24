.PHONY: all run clean memcheck

all:
	cd src ; make

clean:
	cd src ; make clean

run: all
	cd build ; ./game verbose

memcheck: all
	cd build ; valgrind ./game
