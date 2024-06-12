#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightPosition;
uniform float time;

out vec3 fragPosition;
out vec3 fragNormal;
out vec4 lightVSPosition;

void main() {
	vec4 newPosition = vec4(vertexPosition,1.f);
	float sin_t = sin(time*2*3.1415926538);
	newPosition.z += pow(cos( (newPosition.x+1.f)/2.f ),100)*.4* sin_t;
	newPosition.z += pow(cos( (1.f-newPosition.x)/2.f ),25)*.1* sin_t;
	mat4 viewModelMatrix = viewMatrix*modelMatrix;
	vec4 transformedPosition = viewModelMatrix*newPosition;
	gl_Position = projectionMatrix * transformedPosition;
	fragPosition = vec3(transformedPosition/transformedPosition.w);
	fragNormal = mat3(transpose(inverse(viewModelMatrix))) * vertexNormal;
	lightVSPosition = viewMatrix * lightPosition;
}
