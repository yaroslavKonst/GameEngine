.PHONY: all

all: \
	../../../build/TextBox.o \
	../../../build/label.o

../../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
