SC=glslc
SC_OPTS=--target-env=vulkan1.0

.PHONY: shaders outdir

shaders: \
	outdir \
	../../../build/converter \
	spir-v/Shader_vert.spv \
	spir-v/Shader_frag.spv

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
