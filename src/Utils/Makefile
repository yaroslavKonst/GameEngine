.PHONY: all

all: \
	../../build/loader.o \
	../../build/TextFileParser.o \
	../../build/ThreadPool.o \
	../../build/Text.o \
	../../build/CommandLineParser.o \
	../../build/compressor.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<

../../build/Text.o: Text.cpp Text.h
	$(CC) $(CC_OPTS) -I/usr/include/freetype2 $(CC_OBJ) -o $@ $<
