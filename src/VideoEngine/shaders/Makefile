SC=glslc
SC_OPTS=--target-env=vulkan1.0

.PHONY: shaders outdir

shaders: \
	outdir \
	../../../build/converter \
	spir-v/ObjectShader_vert.spv \
	spir-v/ObjectShader_frag.spv \
	spir-v/RectangleShader_vert.spv \
	spir-v/RectangleShader_frag.spv \
	spir-v/SkyboxShader_vert.spv \
	spir-v/SkyboxShader_frag.spv \
	spir-v/ShadowShader_vert.spv \
	spir-v/ShadowShader_geom.spv \
	spir-v/ShadowShader_frag.spv \
	spir-v/PostprocessingShader_vert.spv \
	spir-v/PostprocessingShader_frag.spv \

outdir:
	mkdir -p spir-v

../../../build/converter: converter/main.cpp
	$(CC) $(CC_OPTS) $< -o $@

spir-v/%_vert.spv: %.vert
	$(SC) $(SC_OPTS) $< -o $@_raw
	../../../build/converter $*Vert $@_raw $@
	rm $@_raw

spir-v/%_frag.spv: %.frag
	$(SC) $(SC_OPTS) $< -o $@_raw
	../../../build/converter $*Frag $@_raw $@
	rm $@_raw

spir-v/%_geom.spv: %.geom
	$(SC) $(SC_OPTS) $< -o $@_raw
	../../../build/converter $*Geom $@_raw $@
	rm $@_raw
