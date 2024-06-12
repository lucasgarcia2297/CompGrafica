#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;

uniform float time;
uniform sampler2D colorTexture;
uniform vec4 color01;

// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;

out vec4 fragColor;

#include "funcs/calcPhong.frag"

float rand(vec2 co){
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	vec2 UV = fragTexCoords + (vec2(0.2f, -1.f) * time);
	vec4 tex = texture(colorTexture, UV) + 1.0f;
	
	float noiseGradient = (1 - pow(fragTexCoords.y + 0.25f, 2.0f)) * rand(UV);
	if(noiseGradient <= 0.25) discard;
	
	fragColor = vec4(tex * color01 * noiseGradient);
}

