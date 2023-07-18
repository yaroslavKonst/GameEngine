.PHONY: all

all: \
	../../build/universe.o \
	../../build/ThreadPool.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
