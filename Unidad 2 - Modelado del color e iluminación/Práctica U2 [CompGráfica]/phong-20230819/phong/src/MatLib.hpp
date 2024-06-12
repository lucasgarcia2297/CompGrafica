#ifndef MATLIB_HPP
#define MATLIB_HPP

#include <vector>
#include <string>
#include "Material.hpp"

namespace matlib {
	extern std::vector<std::string> names;
	extern std::vector<Material> materials;
	void add(const std::string &name, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess);
	void init();
} //matlib

#endif
