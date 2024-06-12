#version 330 core

in vec3 vertexPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightPosition;

void main() {
	vec4 modelPos = modelMatrix * vec4(vertexPosition,1.f); 
//	vec3 ldir = normalize(modelPos.xyz/modelPos.w - vec3(lightPosition));
	modelPos -= normalize(lightPosition)*modelPos.y; modelPos.y = 0.01;
	gl_Position = projectionMatrix * viewMatrix * modelPos;
}
