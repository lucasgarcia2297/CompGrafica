#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
uniform float outline_factor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragPosition;
out vec3 fragNormal;

void main() {
	mat4 vm = viewMatrix * modelMatrix;
	vec4 vmp = vm * vec4(vertexPosition+vertexNormal*outline_factor*0.001,1.f);
	fragPosition = vec3(vmp)/vmp.w;
	gl_Position = projectionMatrix * vmp;
	fragNormal = mat3(transpose(inverse(vm))) * vertexNormal;
}
