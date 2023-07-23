.PHONY: all

all: \
	../../build/loader.o \
	../../build/TextFileParser.o \
	../../build/ThreadPool.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
