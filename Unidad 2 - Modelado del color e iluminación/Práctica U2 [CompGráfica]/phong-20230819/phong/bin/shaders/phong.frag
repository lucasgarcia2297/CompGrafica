#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec4 lightVSPosition1;
in vec4 lightVSPosition2;

// propiedades del material
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;
uniform vec3 emissionColor;
uniform float opacity;
uniform float shininess;

// propiedades de la luz
uniform float ambientStrength1;
uniform float ambientStrength2;
uniform vec3 lightColor1;
uniform vec3 lightColor2;

out vec4 fragColor;

#include "funcs/calcPhong.frag"

void main() {
	vec3 phong1 = calcPhong(lightVSPosition1, lightColor1, ambientStrength1,
						   ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong2 = calcPhong(lightVSPosition2, lightColor2, ambientStrength2,
						   ambientColor, diffuseColor, specularColor, shininess);
	fragColor = vec4(phong1+phong2+emissionColor,opacity);
}
