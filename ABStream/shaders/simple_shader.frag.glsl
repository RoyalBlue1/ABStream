#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 uv;
layout (location = 1) flat in uint texId;

layout (location = 0) out uint outBin;


layout(push_constant) uniform Push {
	mat4 transform; // projection * view * model
	int matCount;
} push;


const float STBSP_NOMINAL_TEX_RES = 4096.0f;
const uint MATERIAL_HISTOGRAM_BIN_COUNT = 16;
void main() {

	vec2 dx = dFdxFine(uv) * STBSP_NOMINAL_TEX_RES;
	vec2 dy = dFdyFine(uv) * STBSP_NOMINAL_TEX_RES;
	float dot = max(dot(dx,dx),dot(dy,dy));

	float mipLevel = floor(clamp( 0.5 * log2(dot), 0.0f, float(MATERIAL_HISTOGRAM_BIN_COUNT - 1) ));

	

	uint bin = MATERIAL_HISTOGRAM_BIN_COUNT * texId + uint(mipLevel);
	outBin = bin;
}