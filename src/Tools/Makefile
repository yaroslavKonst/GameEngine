.PHONY: all

all: \
	$(BUILD_DIR)/packer \
	$(BUILD_DIR)/SoundTool \
	$(BUILD_DIR)/SoftBodyTool

$(BUILD_DIR)/packer: packer.cpp $(BUILD_DIR)/engine.a
	$(CXX) $(CXX_OPTS) -o $@ $< $(BUILD_DIR)/engine.a

$(BUILD_DIR)/SoundTool: SoundTool.cpp $(BUILD_DIR)/engine.a
	$(CXX) $(CXX_OPTS) -fopenmp -o $@ $< $(BUILD_DIR)/engine.a $(LIBS)

$(BUILD_DIR)/SoftBodyTool: SoftBodyTool.cpp $(BUILD_DIR)/engine.a
	$(CXX) $(CXX_OPTS) -o $@ $< $(BUILD_DIR)/engine.a $(LIBS)
