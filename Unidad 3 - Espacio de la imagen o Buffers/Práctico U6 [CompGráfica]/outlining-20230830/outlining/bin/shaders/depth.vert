#version 330 core

in vec3 vertexPosition;
in vec2 vertexTexCoords;

out vec2 fragTexCoords;

void main() {
	gl_Position = vec4(vertexPosition,1.f);
	fragTexCoords = vertexTexCoords;
}
