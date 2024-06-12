#version 330 core

// propiedades del material
uniform vec3 objectColor;
uniform float shininess;
uniform float specularStrength;

// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;
uniform vec4 lightPosition;

// propiedades de la camara
uniform vec3 cameraPosition;

// propiedades del fragmento interpoladas por el vertex shader
in vec3 fragNormal;
in vec3 fragPosition;

// resultado
out vec4 fragColor;

// phong simplificado
void main() {
	
	// ambient
	vec3 ambient = lightColor * ambientStrength  * objectColor ;
	//Ia = Ia*Ka
	//donde: 
	// Ia = lightColor*ambientStrength - Ka = objectColor 
	
	// diffuse
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(vec3(lightPosition)); // luz directional (en el inf.)
	vec3 diffuse =   lightColor* objectColor * max(dot(norm,lightDir),0.f);
	//Is = Id*Kd*(cos(Tita))
	//donde: 
	//   Ii = lightColor - Kd = objectColor - cos(tita) = dot(lightDir,norm)
	
	
	// specular
	vec3 specularColor = specularStrength * vec3(1.f,1.f,1.f);
	vec3 viewDir = normalize(cameraPosition-fragPosition);
	vec3 halfV = normalize(lightDir + viewDir); // blinn
	vec3 specular = lightColor * specularColor * pow(max(dot(norm,halfV),0.f),shininess);
	//Is = Ii*Ks*(cos(B))^q
	//donde: 
	//   Ii = lightColor - Ks = specularStrength - cos(B) = dot(norm,halfv) - q = shininess
	
	
	// result
	fragColor = vec4(ambient+diffuse+specular,1.f);
}

