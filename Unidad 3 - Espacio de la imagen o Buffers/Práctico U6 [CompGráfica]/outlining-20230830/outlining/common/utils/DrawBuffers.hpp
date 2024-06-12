#ifndef DRAWBUFFERS_HPP
#define DRAWBUFFERS_HPP

#include "Shaders.hpp"

class GLFWwindow;

class DrawBuffers {
public:
//	DrawBuffers(); // usamos inicializacion lazy para el caso en que se declara global (seguramente todavia no hay contexto al contruir)
	void init();
	void drawStencil(int max);
	void drawDepth(int w, int h, float exp=2.f);
	void addImGuiSettings(GLFWwindow *window);
	void draw(int w, int h);
	void setNextBuffer();
	~DrawBuffers();
private:
	Shader &setShaderAndVBOs(bool stencil);
	GLuint VAO=0, VBO[2]={0,0}, tex_id=0;
	Shader shader_stencil;
	Shader shader_depth;
	// para ImGui
	int buffer_id = 0; float depht_exp = 3.f;
	const std::vector<std::string> vbuffers = { "Color", "Stencil", "Depth" };
};

#endif

