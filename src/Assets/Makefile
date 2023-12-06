.PHONY: all package

all: \
	../../build/animation.o \
	../../build/ScriptHandler.o \
	../../build/package.o \
	../../build/localizer.o \
	package

package: ../../build/packer
	cd Resources ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)

../../build/packer: packer.cpp ../../build/package.o ../../build/compressor.o
	$(CC) $(CC_OPTS) -o $@ $< ../../build/package.o ../../build/compressor.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
