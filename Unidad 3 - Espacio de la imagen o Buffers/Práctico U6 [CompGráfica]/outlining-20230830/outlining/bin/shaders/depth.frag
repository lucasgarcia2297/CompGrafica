#version 330 core

in vec2 fragTexCoords;

uniform sampler2D depthData;
uniform float exp;

out vec4 fragColor;

void main() {
	float d = texture(depthData,fragTexCoords).r;
	float dp = pow(d,exp);
	float r = dp, g = dp, b = dp;
	if (d==1.f) r = b = 0.f;
	if (d<0.01f) r = 1.f;
	fragColor = vec4(r,g,b,1.f);
}

