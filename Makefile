.PHONY: all run clean memcheck

all:
	cd src ; make

clean:
	cd src ; make clean

run: all
	cd build ; ./game verbose colored_log

memcheck: all
	cd build ; valgrind ./game
