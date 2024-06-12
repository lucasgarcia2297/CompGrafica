#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec4 lightVSPosition1;
in vec4 lightVSPosition2;

// propiedades del material
uniform vec3 diffuseColor;
uniform vec3 baseReflectivity;
uniform float metallic;
uniform float roughness;

// propiedades de la luz
uniform vec3 lightColor1;
uniform vec3 lightColor2;
uniform float ambientStrength1;
uniform float ambientStrength2;

out vec4 fragColor;

#include "funcs/calcPBR.frag"

void main() {
	vec3 pbr1 = calcPBR(lightVSPosition1, lightColor1, ambientStrength1,
						metallic, roughness, diffuseColor, baseReflectivity);
	vec3 pbr2 = calcPBR(lightVSPosition2, lightColor2, ambientStrength2,
						metallic, roughness, diffuseColor, baseReflectivity);
	vec3 sum = pbr1+pbr2/*+emissionColor*/;
	
	vec3 hdr = sum/(sum+vec3(1.0));
//	vec3 gamma = pow(hdr,vec3(1.0/2.2)); // ya lo hace opengl con GL_FRAMEBUFFER_SRGB
	fragColor = vec4(hdr,1.0);
}
