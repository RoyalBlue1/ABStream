#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in uint texId;


layout(location = 0) out vec2 uvOut;
layout(location = 1) out uint texIdOut;



layout(push_constant) uniform Push {
	mat4 transform; // projection * view * model
	int matCount;
} push;


void main(){
	gl_Position = push.transform * vec4(position,1.0);


	uvOut = uv;
	texIdOut = texId;
	
}