.PHONY: all run clean

all:
	cd src ; make

clean:
	cd src ; make clean

run: all
	cd build ; ./game
