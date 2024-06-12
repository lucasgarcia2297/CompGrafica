#include "MatLib.hpp"

namespace matlib {
	std::vector<std::string> names;
	std::vector<Material> materials;
	void add(const std::string &name, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess, float opacity) {
		Material mat;
		mat.ka = ambient;
		mat.kd = diffuse;
		mat.ks = specular;
		mat.ke = glm::vec3(0.f,0.f,0.f);
		mat.opacity = opacity;
		mat.shininess = shininess;
		names.push_back(name);
		materials.push_back(mat);
	}
	void init() {
		// tomados de la tabla 3.1 del Advanced Graphics Programming Using OpenGL, Tom McReynolds and David Blythe, Morgan Kaufmann Publishers, 2005. ??
		add(" ",                { 0.0f     , 0.0f     , 0.0f      },  { 0.0f     , 0.0f     , 0.0f      },  { 0.0f     , 0.0f     , 0.0f      },   1.0     ,1.0f );
		add("Brass",            { 0.329412f, 0.223529f, 0.027451f },  { 0.780392f, 0.568627f, 0.113725f },  { 0.992157f, 0.941176f, 0.807843f },  27.8974f ,1.0f );
		add("Bronze",           { 0.2125f,   0.1275f,   0.054f    },  { 0.714f,    0.4284f,   0.18144f  },  { 0.393548f, 0.271906f, 0.166721f },  25.6f    ,1.0f );
		add("Polished_Bronze",  { 0.25f,     0.148f,    0.06475f  },  { 0.4f,      0.2368f,   0.1036f   },  { 0.774597f, 0.458561f, 0.200621f },  76.8f    ,1.0f );
		add("Chrome",           { 0.25f,     0.25f,     0.25f     },  { 0.4f,      0.4f,      0.4f      },  { 0.774597f, 0.774597f, 0.774597f },  76.8f    ,1.0f );
		add("Copper",           { 0.19125f,  0.0735f,   0.0225f   },  { 0.7038f,   0.27048f,  0.0828f   },  { 0.256777f, 0.137622f, 0.086014f },  12.8f    ,1.0f );
		add("Polished_Copper",  { 0.2295f,   0.08825f,  0.0275f   },  { 0.5508f,   0.2118f,   0.066f    },  { 0.580594f, 0.223257f, 0.0695701f},  51.2f    ,1.0f );
		add("Black_Rubber",     { 0.02f,     0.02f,     0.02f     },  { 0.01f,     0.01f,     0.01f     },  { 0.4f,      0.4f,      0.4f      },  10.0f    ,1.0f );
		add("Gold",             { 0.24725f,  0.1995f,   0.0745f   },  { 0.75164f,  0.60648f,  0.22648f  },  { 0.628281f, 0.555802f, 0.366065f },  51.2f    ,1.0f );
		add("Polished_Gold",    { 0.24725f,  0.2245f,   0.0645f   },  { 0.34615f,  0.3143f,   0.0903f   },  { 0.797357f, 0.723991f, 0.208006f },  83.2f    ,1.0f );
		add("Pewter",           { 0.105882f, 0.058824f, 0.113725f },  { 0.427451f, 0.470588f, 0.541176f },  { 0.333333f, 0.333333f, 0.521569f },   9.84615f,1.0f );
		add("Silver",           { 0.19225f,  0.19225f,  0.19225f  },  { 0.50754f,  0.50754f,  0.50754f  },  { 0.508273f, 0.508273f, 0.508273f },  51.2f    ,1.0f );
		add("Polished_Silver",  { 0.23125f,  0.23125f,  0.23125f  },  { 0.2775f,   0.2775f,   0.2775f   },  { 0.773911f, 0.773911f, 0.773911f },  89.6f    ,1.0f );
		add("Emerald",          { 0.0215f,   0.1745f,   0.0215f   },  { 0.07568f,  0.61424f,  0.07568f  },  { 0.633f,    0.727811f, 0.633f    },  76.8f    ,0.55f );
		add("Jade",             { 0.135f,    0.2225f,   0.1575f   },  { 0.54f,     0.89f,     0.63f     },  { 0.316228f, 0.316228f, 0.316228f },  12.8f    ,0.95f );
		add("Obsidian",         { 0.05375f,  0.05f,     0.06625f  },  { 0.18275f,  0.17f,     0.22525f  },  { 0.332741f, 0.328634f, 0.346435f },  38.4f    ,0.82f );
		add("Pearl",            { 0.25f,     0.20725f,  0.20725f  },  { 1.0f,      0.829f,    0.829f    },  { 0.296648f, 0.296648f, 0.296648f },  11.264f  ,0.992f );
		add("Ruby",             { 0.1745f,   0.01175f,  0.01175f  },  { 0.61424f,  0.04136f,  0.04136f  },  { 0.727811f, 0.626959f, 0.626959f },  76.8f    ,0.55f );
		add("Turquoise",        { 0.1f,      0.18725f,  0.1745f   },  { 0.396f,    0.74151f,  0.69102f  },  { 0.297254f, 0.30829f,  0.306678f },  12.8f    ,0.8f );
		add("Black_Plastic",    { 0.0f,      0.0f,      0.0f      },  { 0.01f,     0.01f,     0.01f     },  { 0.5f,      0.5f,      0.5f      },  32.0f    ,1.0f );
	}
} //matlib
