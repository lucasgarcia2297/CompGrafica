#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoords;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightPosition;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoords;
out vec4 lightVSPosition;

void main() {
	mat4 viewModel = viewMatrix * modelMatrix;
	vec4 vertexPosWorld = viewModel * vec4(vertexPosition,1.f);
	gl_Position = projectionMatrix * vertexPosWorld;
	fragPosition = vec3(vertexPosWorld);
	fragNormal = mat3(transpose(inverse(viewModel))) * vertexNormal;
	lightVSPosition = viewMatrix * lightPosition;
	fragTexCoords = vertexTexCoords;
}
