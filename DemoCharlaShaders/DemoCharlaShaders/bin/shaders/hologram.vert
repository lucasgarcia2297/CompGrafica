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

// user parameters
uniform float time;

void main() {
	float newTime = time;
	
	// 1) Modify VertexPosition to apply a sine movement to Y coord
	vec3 newPos= vec3(vertexPosition.x*cos(newTime) + vertexPosition.z*sin(newTime), 
					  vertexPosition.y + 0.2f*sin(5*newTime),
					  vertexPosition.z*cos(newTime) - vertexPosition.x*sin(newTime));
	
	// 2) Rotate all vertex around Y axis
	// 3) define a new variable using a float Uniform to controll the animation speed
	
	mat4 vm = viewMatrix * modelMatrix;
	vec4 vmp = vm * vec4(newPos, 1.f);
	
	gl_Position = projectionMatrix * vmp;
	fragPosition = vec3(vmp);
	fragNormal = mat3(transpose(inverse(vm))) * vertexNormal;
	lightVSPosition = viewMatrix * lightPosition;
	fragTexCoords = vertexTexCoords;
}
