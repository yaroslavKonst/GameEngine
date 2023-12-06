.PHONY: all

all: \
	../../build/SoundTool

../../build/SoundTool: SoundTool.cpp ../../build/audio.o ../../build/logger.o ../../build/TextFileParser.o ../../build/package.o ../../build/compressor.o
	$(CC) $(CC_OPTS) -fopenmp -o $@ $< ../../build/audio.o ../../build/logger.o ../../build/TextFileParser.o ../../build/package.o ../../build/compressor.o -lportaudio

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
