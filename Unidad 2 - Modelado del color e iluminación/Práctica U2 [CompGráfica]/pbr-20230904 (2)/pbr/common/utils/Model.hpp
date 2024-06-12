#ifndef MODEL_HPP
#define MODEL_HPP
#include <vector>
#include "Geometry.hpp"
#include "Material.hpp"
#include "Texture.hpp"

// auxiliar struct for loading all model-related data
struct Model {
	Geometry geometry;
	GeometryRenderer buffers;
	Material material;
	Texture texture;
	
	Model() = default;
	
	Model(Geometry &&g, const Material &m, int flags) 
		: buffers(g,flags&fDynamic), material(m), 
		  texture(m.texture.empty() or (flags&fNoTextures) 
	           ? Texture() 
			   : Texture(m.texture, model2texture(flags)) )
	{
		if (flags&fKeepGeometry) geometry = std::move(g);
	}
	
	// flags meanings are such that 0 is default behaviour and it matches 
	// what obj (texture repeat and flipV) and most examples (fit, static data 
	// and discard geometry) expects
	enum Flags { fNone=0, fDontFit=1, fKeepGeometry=2, 
				 fRegenerateNormals=4, fDynamic=8, 
		         fNoTextures=16, fTextureDontFlipV=32, fTextureClamp=64 };
	static std::vector<Model> load(const std::string &name, int flags = 0);
	static Model loadSingle(const std::string &name, int flags = 0);
	
private:
	static int model2texture(int flags) {
		return 
			((flags&fTextureClamp)?(Texture::fClampS|Texture::fClampT):0)
			| ((flags&fTextureDontFlipV)?Texture::fY0OnTop:0);
	}
};

void centerAndResize(std::vector<glm::vec3> &v);

#endif

