#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;

// propiedades del material
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;
uniform vec3 emissionColor;
uniform float opacity;
uniform float shininess;
// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;


//User shader parameters
uniform float time;		
uniform float freq;	
uniform float tresh;
uniform float sine_speed;
uniform float test04;

uniform sampler2D chookity_tex;
uniform sampler2D noise_tex;

uniform vec2 uv_scale;
uniform vec2 uv_scroll;

uniform vec4 color01;

// Shader output
out vec4 fragColor;

float rand(vec2 co){
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
	
void main() {

	// 1) Sample textures
	vec2 newUV = fragTexCoords * uv_scale+ time * uv_scroll;
	vec4 chookColor = texture(chookity_tex, fragTexCoords);
	vec4 noiseColor = texture(noise_tex, newUV);
	// 1.5) Scroll texture
	
	
	// 2) Interpolate colors
	vec3 colorMix = mix(chookColor.rgb,noiseColor.rgb, tresh);
	// 3) Draw sine
	float chookSin = sin(freq* 10.f * newUV.y + time*sine_speed);

	// 4) Discard pixels using sine value
	if(chookSin<tresh) discard;
	
	// 5) Use rand() function to add noise
	
	
	
	vec3 finalColor = colorMix.rgb*color01.rgb + rand(fragTexCoords+time)*0.2f;
	
	
	fragColor = vec4(finalColor.rgb,1.f);
	
//	fragColor = vec4(0.f,0.f,0.0f, 1.0f);
}

