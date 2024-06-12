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
	vec3 ambient = lightColor * ambientStrength * objectColor ;
	
	// diffuse
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(vec3(lightPosition)); // luz directional (en el inf.)
	float cosTita = dot(norm,lightDir);
	
	vec3 diffuse = lightColor*objectColor;
//	vec3 diffuse = vec3(0.2f,0.2f,0.2f);
	
	// specular
	vec3 specularColor = specularStrength * vec3(1.f,1.f,1.f);
	vec3 viewDir = normalize(cameraPosition-fragPosition);
	vec3 halfV = normalize(lightDir + viewDir); // blinn
	float cosBeta = pow(max(dot(norm,halfV),0.f),shininess);
	vec3 specular = lightColor * specularColor;
	
	
	
	if(cosTita < 0.3){
		diffuse = diffuse*0.f;
	}else if(cosTita<0.7){
		diffuse = diffuse*0.5f;
	}else{
		diffuse = diffuse*1.f;
	}
	
	
	if(cosBeta < 0.5){
		specular = specular*0.f;
	}else{
		specular = specular*1.f;
	}
	// result
	fragColor = vec4(ambient+diffuse+specular,1.f);
		
}

