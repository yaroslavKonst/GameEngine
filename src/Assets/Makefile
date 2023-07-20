.PHONY: all

all: \
	../../build/triangle.o \
	../../build/square.o

shaders:
	cd shaders ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
