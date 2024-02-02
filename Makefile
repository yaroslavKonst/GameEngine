.PHONY: all run clean

export BUILD_DIR != echo `pwd`/build

all:
	cd src ; make

clean:
	cd src ; make clean

run: all
	cd build ; ./game verbose colored_log
