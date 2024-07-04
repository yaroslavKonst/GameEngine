export PREFIX = $(BUILD_DIR)/Engine

SC = glslc
SC_OPTS = --target-env=vulkan1.0

VIDEO_SOURCES := $(shell ls Video/*.cpp)
VIDEO_OBJECTS := $(VIDEO_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS = $(VIDEO_OBJECTS)

GUI_SOURCES := $(shell ls Video/GUI/*.cpp)
GUI_OBJECTS := $(GUI_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(GUI_OBJECTS)

AUDIO_SOURCES := $(shell ls Audio/*.cpp)
AUDIO_OBJECTS := $(AUDIO_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(AUDIO_OBJECTS)

TIME_SOURCES := $(shell ls Time/*.cpp)
TIME_OBJECTS := $(TIME_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(TIME_OBJECTS)

PHYSICS_SOURCES := $(shell ls Physics/*.cpp)
PHYSICS_OBJECTS := $(PHYSICS_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(PHYSICS_OBJECTS)

UTILS_SOURCES := $(shell ls Utils/*.cpp)
UTILS_OBJECTS := $(UTILS_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(UTILS_OBJECTS)

LOGGER_SOURCES := $(shell ls Logger/*.cpp)
LOGGER_OBJECTS := $(LOGGER_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(LOGGER_OBJECTS)

ASSETS_SOURCES := $(shell ls Assets/*.cpp)
ASSETS_OBJECTS := $(ASSETS_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(ASSETS_OBJECTS)

SYNC_SOURCES := $(shell ls Sync/*.cpp)
SYNC_OBJECTS := $(SYNC_SOURCES:%.cpp=$(PREFIX)/%.o)
OBJECTS += $(SYNC_OBJECTS)

$(BUILD_DIR)/engine.a: $(OBJECTS) | $(PREFIX)
	$(AR) $@ $(OBJECTS)

$(PREFIX):
	mkdir -p $@

# Video
VIDEO_PREFIX = $(PREFIX)/Video

.PHONY: shaders

$(VIDEO_OBJECTS): | $(VIDEO_PREFIX)

$(VIDEO_PREFIX):
	mkdir -p $@

$(VIDEO_PREFIX)/swapchain.o: \
	$(VIDEO_PREFIX)/spir-v/ObjectShader_vert.spv \
	$(VIDEO_PREFIX)/spir-v/ObjectShader_frag.spv \
	$(VIDEO_PREFIX)/spir-v/ObjectShaderDiscard_frag.spv \
	$(VIDEO_PREFIX)/spir-v/RectangleShader_vert.spv \
	$(VIDEO_PREFIX)/spir-v/RectangleShader_frag.spv \
	$(VIDEO_PREFIX)/spir-v/SkyboxShader_vert.spv \
	$(VIDEO_PREFIX)/spir-v/SkyboxShader_frag.spv \
	$(VIDEO_PREFIX)/spir-v/ShadowShader_vert.spv \
	$(VIDEO_PREFIX)/spir-v/ShadowShader_geom.spv \
	$(VIDEO_PREFIX)/spir-v/ShadowShader_frag.spv \
	$(VIDEO_PREFIX)/spir-v/ShadowShaderDiscard_vert.spv \
	$(VIDEO_PREFIX)/spir-v/ShadowShaderDiscard_geom.spv \
	$(VIDEO_PREFIX)/spir-v/ShadowShaderDiscard_frag.spv \
	$(VIDEO_PREFIX)/spir-v/PostprocessingShader_vert.spv \
	$(VIDEO_PREFIX)/spir-v/PostprocessingShader_frag.spv \
	$(VIDEO_PREFIX)/spir-v/SpriteShader_vert.spv \
	$(VIDEO_PREFIX)/spir-v/SpriteShader_frag.spv

$(VIDEO_PREFIX)/spir-v:
	mkdir -p $@

$(VIDEO_PREFIX)/converter: Video/shaders/converter/main.cpp
	$(CXX) $(CXX_OPTS) $< -o $@

$(VIDEO_PREFIX)/spir-v/%_vert.spv: Video/shaders/%.vert Video/shaders/DepthTransform.glsl $(VIDEO_PREFIX)/converter | $(VIDEO_PREFIX)/spir-v
	$(SC) $(SC_OPTS) $< -o $@_raw
	$(VIDEO_PREFIX)/converter $*Vert $@_raw $@
	rm $@_raw

$(VIDEO_PREFIX)/spir-v/%_frag.spv: Video/shaders/%.frag Video/shaders/shadow.glsl Video/shaders/LightProcessing.glsl $(VIDEO_PREFIX)/converter | $(VIDEO_PREFIX)/spir-v
	$(SC) $(SC_OPTS) $< -o $@_raw
	$(VIDEO_PREFIX)/converter $*Frag $@_raw $@
	rm $@_raw

$(VIDEO_PREFIX)/spir-v/%_geom.spv: Video/shaders/%.geom $(VIDEO_PREFIX)/converter | $(VIDEO_PREFIX)/spir-v
	$(SC) $(SC_OPTS) $< -o $@_raw
	$(VIDEO_PREFIX)/converter $*Geom $@_raw $@
	rm $@_raw

# GUI
$(GUI_OBJECTS): | $(VIDEO_PREFIX)/GUI

$(VIDEO_PREFIX)/GUI:
	mkdir -p $@

# Audio
$(AUDIO_OBJECTS): | $(PREFIX)/Audio

$(PREFIX)/Audio:
	mkdir -p $@

# Time
$(TIME_OBJECTS): | $(PREFIX)/Time

$(PREFIX)/Time:
	mkdir -p $@

# Physics
$(PHYSICS_OBJECTS): | $(PREFIX)/Physics

$(PREFIX)/Physics:
	mkdir -p $@

# Utils
$(UTILS_OBJECTS): | $(PREFIX)/Utils

$(PREFIX)/Utils:
	mkdir -p $@

$(PREFIX)/Utils/Text.o: Utils/Text.cpp Utils/Text.h
	$(CXX) $(CXX_OPTS) $(CXX_OBJ) -I/usr/include/freetype2 -o $@ $<

# Logger
$(LOGGER_OBJECTS): | $(PREFIX)/Logger

$(PREFIX)/Logger:
	mkdir -p $@

# Assets
$(ASSETS_OBJECTS): | $(PREFIX)/Assets

$(PREFIX)/Assets:
	mkdir -p $@

# Sync
$(SYNC_OBJECTS): | $(PREFIX)/Sync

$(PREFIX)/Sync:
	mkdir -p $@

# Object build
$(PREFIX)/%.o: %.cpp %.h
	$(CXX) $(CXX_OPTS) $(CXX_OBJ) -o $@ $<
