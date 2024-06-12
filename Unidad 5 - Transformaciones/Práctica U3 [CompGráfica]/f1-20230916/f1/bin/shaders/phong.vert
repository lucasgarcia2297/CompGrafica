#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightPosition;

out vec3 fragPosition;
out vec3 fragNormal;
out vec4 lightVSPosition;

void main() {
	mat4 viewModel = viewMatrix*modelMatrix;
	vec4 pAux = viewModel * vec4(vertexPosition,1.f);
	gl_Position = projectionMatrix * pAux;
	fragPosition = pAux.xyz/pAux.w;
	mat3 normalMat = mat3(transpose(inverse(viewModel)));
	fragNormal = normalMat * normalize(vertexNormal);
	fragNormal = determinant(viewModel)<0 ? -fragNormal : fragNormal;
	lightVSPosition = viewMatrix * lightPosition;
}
