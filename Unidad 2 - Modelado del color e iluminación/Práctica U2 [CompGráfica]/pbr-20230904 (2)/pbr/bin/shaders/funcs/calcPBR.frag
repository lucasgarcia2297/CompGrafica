// based on https://www.youtube.com/watch?v=XK_p2MxGBQs
// and https://learnopengl.com/PBR/Lighting
	
#define PI 3.1415926538
vec3 calcPBR(vec4 lightPosition, vec3 lightColor, float ambientStrength,
			 float metallic, float roughness, vec3 diffuseColor, vec3 baseReflectivity)
{
	vec3 normal = normalize(gl_FrontFacing? fragNormal : -fragNormal);
	vec3 lightDir = normalize(vec3(lightPosition)-fragPosition*lightPosition.w); // w=0 => directional
	vec3 viewDir = normalize(-fragPosition);
	vec3 halfV = normalize(lightDir + viewDir);
	float normDotLight = max(dot(normal,lightDir),0.f);
	float normDotView = max(dot(normal,viewDir),0.f);
	float normDotHalf = max(dot(normal,halfV),0.f);
	
	//	vec3 specularBRDF = specularColor*cookTorrance;
	// normal distribution function: GGX (Trowbridge & Reitz)
	float alpha = roughness*roughness;
	float alpha2 = alpha*alpha;
	float aux_nd = (normDotHalf*normDotHalf) * (alpha2-1.0) + 1.0;
	float normalDistribution = alpha2 / (PI * aux_nd * aux_nd);
	
	// geometry function: Schlick-GGX (Schlick & Beckmann)
	float k = (roughness+1.0)*(roughness+1.0) / 8.0;
	float geometryView = normDotView / (normDotView*(1.0-k) + k);
	float geometryLight = normDotLight / (normDotLight*(1.0-k) + k);
	float geometry = geometryView*geometryLight; // Smith method
	
	// fresnel function: Fresnel Schlikc
	vec3 F0 = mix(vec3(0.04),baseReflectivity,metallic);
	float aux_fre = clamp(1.0-dot(viewDir,halfV),0.0,1.0);
	vec3 fresnel = F0 + (1.0-F0)*(aux_fre*aux_fre*aux_fre*aux_fre*aux_fre);
	
	// diffuse (lambert)
	vec3 lambert = diffuseColor / PI;
	// diffuse (oren-nayer)
	float A = 1.0 - 0.5*alpha/(alpha+0.33);
	float B = 0.45*alpha/(alpha+0.09);
	float ga = dot(viewDir-normal*normDotView,normal-normal*normDotLight);
	vec3 oren_nayar = lambert * (A + B * max(0., ga) * sqrt(max((1.0-normDotView*normDotView)*(1.0-normDotLight*normDotLight), 0.)) / max(normDotLight, normDotView));
	
	// final color
	vec3 ks = fresnel, kd = (1.0-fresnel)*(1.0-metallic);
	vec3 diffuseBRDF = kd * oren_nayar;
	vec3 specularBRDF = (normalDistribution*geometry*fresnel)/(4.0*normDotLight*normDotView+0.00001);
	vec3 ambient = (ambientStrength*lightColor)*diffuseColor*kd;
	return (diffuseBRDF + specularBRDF)*lightColor*normDotLight+ambient;
}
