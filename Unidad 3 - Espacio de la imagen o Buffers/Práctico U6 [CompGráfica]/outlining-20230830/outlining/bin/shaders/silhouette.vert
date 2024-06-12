#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform float outline_width;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float time;

void main() {
	//Acá es donde se da el inflado de la silueta
	mat4 vm = viewMatrix * modelMatrix;			
//	vec4 vmp = vm * vec4(vertexPosition+vertexNormal*outline_width*sin(5*time) ,1.f); //Para probar
	vec4 vmp = vm * vec4(vertexPosition+vertexNormal*outline_width ,1.f);
	gl_Position = projectionMatrix * vmp;
}
