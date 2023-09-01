.PHONY: all

all: \
	../../../build/TextBox.o

../../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
