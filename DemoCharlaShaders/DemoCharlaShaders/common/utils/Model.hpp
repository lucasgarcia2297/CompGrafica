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
	Texture texture01;
	Texture texture02;
	Texture texture03;
	
	Model() = default;
	Model(const Geometry &g, const Material &m) 
		: buffers(g), material(m), 
		  texture01(m.texture01.empty() ? Texture() : Texture(m.texture01)),
		texture02(m.texture02.empty() ? Texture() : Texture(m.texture02)),
		texture03(m.texture03.empty() ? Texture() : Texture(m.texture03))
	{
		
	}
	Model(Geometry &&g, const Material &m, bool keep_geometry=false) 
		: buffers(g), material(m), 
		texture01(m.texture01.empty() ? Texture() : Texture(m.texture01)),
		texture02(m.texture02.empty() ? Texture() : Texture(m.texture02)),
		texture03(m.texture03.empty() ? Texture() : Texture(m.texture03))
	{
		if (keep_geometry) geometry = std::move(g);
	}
	
	enum Flags { fNone=0, fDontFit=1, fKeepGeometry=2, 
				 fRegenerateNormals=4, fDynamic=8, fNoTextures=16 };
	static std::vector<Model> load(const std::string &name, int flags = 0);
	static Model loadSingle(const std::string &name, int flags = 0);
};

void centerAndResize(std::vector<glm::vec3> &v);

#endif

