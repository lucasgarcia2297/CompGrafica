#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightPosition1;
uniform vec4 lightPosition2;

out vec3 fragPosition;
out vec3 fragNormal;
out vec4 lightVSPosition1;
out vec4 lightVSPosition2;

void main() {
	mat4 vm = viewMatrix * modelMatrix;
	vec4 vmp = vm * vec4(vertexPosition,1.f);
	fragPosition = vec3(vmp)/vmp.w;
	gl_Position = projectionMatrix * vmp;
	fragNormal = mat3(transpose(inverse(vm))) * vertexNormal;
	lightVSPosition1 = viewMatrix * lightPosition1;
	lightVSPosition2 = viewMatrix * lightPosition2;
}
