#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;

// propiedades del material
uniform sampler2D colorTexture;
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;
uniform vec3 emissionColor;
uniform float opacity;
uniform float shininess;

// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;

out vec4 fragColor;

#include "funcs/calcPhong.frag"

void main() {
	vec4 tex = texture(colorTexture,fragTexCoords);
	vec3 phong = calcPhong(lightVSPosition, lightColor,
						   mix(ambientColor,vec3(tex),tex.a),
						   mix(diffuseColor,vec3(tex),tex.a),
						   specularColor, shininess);
//	fragColor = vec4(phong+emissionColor,opacity);
	//	fragColor = vec4(emissionColor+vec3(colorTexture+fragTexCoords,0.f),opacity);
//	fragColor = vec4(vec3(fragTexCoords.x,fragTexCoords.y,0.f),1.f);
	fragColor = vec4(vec3(fragTexCoords.s-floor(fragTexCoords.s),0.f,0.f),opacity);
}

