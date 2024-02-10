PREFIX = $(BUILD_DIR)/Game

OBJECTS = \
	$(PREFIX)/player.o \
	$(PREFIX)/world.o \
	$(PREFIX)/shuttle.o \
	$(PREFIX)/planet.o \
	$(PREFIX)/PhysTest.o \
	$(PREFIX)/main.o

$(BUILD_DIR)/game: $(OBJECTS) $(BUILD_DIR)/engine.a
	$(CXX) -o $@ $(OBJECTS) $(BUILD_DIR)/engine.a $(LIBS)

$(PREFIX)/main.o: main.cpp | $(PREFIX)
	$(CXX) $(CXX_OPTS) $(CXX_OBJ) -o $@ $<

$(PREFIX)/%.o: %.cpp %.h | $(PREFIX)
	$(CXX) $(CXX_OPTS) $(CXX_OBJ) -o $@ $<

$(PREFIX):
	mkdir -p $@
