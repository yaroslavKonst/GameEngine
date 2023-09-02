.PHONY: all package

all: \
	../../build/square.o \
	../../build/animation.o \
	../../build/ScriptHandler.o \
	../../build/package.o \
	package

package: ../../build/packer
	cd Resources ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)

../../build/packer: packer.cpp ../../build/package.o
	$(CC) $(CC_OPTS) -o $@ $< ../../build/package.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
