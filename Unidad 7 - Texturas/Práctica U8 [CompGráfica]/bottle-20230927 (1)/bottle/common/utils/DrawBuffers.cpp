#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Window.hpp"
#include "Callbacks.hpp"
#include "DrawBuffers.hpp"
#include "Debug.hpp"

static glm::vec4 hsv2rgb(float h, float s, float v, float a) {
	
	if(h >= 360.f) h = 0.f; else h /= 60.f;
	
	int i = (int)h;
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - (s * f));
	float t = v * (1.f - (s * (1.f - f)));
	
	switch(i) {
	case 0:  return {v,t,p,a};
	case 1:  return {q,v,p,a};
	case 2:  return {p,v,t,a};
	case 3:  return {p,q,v,a};
	case 4:  return {t,p,v,a};
	default: return {v,p,q,a};
	}
}

static glm::vec4 getColor(int i) {
	constexpr float golden_ration = 1.61803398875;
	float hue = std::fmod(i*360.f/golden_ration,360.f);
	return hsv2rgb(hue,.7f,1.f,0.75f);
}

void DrawBuffers::init() {
	cg_assert(VAO==0,"DrawBuffers already initialized");
	shader_stencil = Shader("shaders/stencil");
	shader_depth = Shader("shaders/depth");
	glGenVertexArrays(1,&VAO);
	glBindVertexArray(VAO);
	std::vector<glm::vec3> vpos = {
		{-1.f,-1.f,0.f}, {+1.f,-1.f,0.f},
		{+1.f,+1.f,0.f}, {-1.f,+1.f,0.f} };
	std::vector<glm::vec2> vtc = {
		{0.f,0.f}, {1.f,0.f},
		{1.f,1.f}, {0.f,1.f} };
	glGenBuffers(2, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vpos.size()*sizeof(glm::vec3), vpos.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, vpos.size()*sizeof(glm::vec2), vtc.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
}

void DrawBuffers::drawStencil(int max) {
	if (VAO==0) init();
	
	glBindVertexArray(VAO);
	Shader &shader = setShaderAndVBOs(true);
	
	bool depth_was_on = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	for(int ref=0;ref<max;++ref) {
		shader.setUniform("color",getColor(ref));
		glStencilFunc(GL_EQUAL,ref,255);
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
	}
	glDisable(GL_STENCIL_TEST);
	if (depth_was_on) glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

DrawBuffers::~DrawBuffers() {
	if (VAO==0) return;
	glDeleteBuffers(2,VBO);
	glDeleteVertexArrays(1,&VAO);
}

static int getStencilValueUnderMouseCursor(GLFWwindow *window) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	unsigned char s;
	glReadPixels(static_cast<int>(x),win_height-static_cast<int>(y),1,1,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,&s);
	return s;
}

void DrawBuffers::drawDepth (int w, int h, float exp) {
	if (VAO==0) init();
	
	glBindVertexArray(VAO);
	
	// capture
	glReadBuffer(GL_DEPTH);
	glActiveTexture(GL_TEXTURE0);
	if (tex_id==0) {
		glGenTextures(1,&tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	} else {
		glBindTexture(GL_TEXTURE_2D, tex_id);
	}
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0,0,w,h, 0);
	glBindTexture(GL_TEXTURE_2D, 0); 
	
	// draw
	Shader &shader = setShaderAndVBOs(false);
	shader.setUniform("exp",exp);
	
	bool depth_was_on = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glDrawArrays(GL_TRIANGLE_FAN, 0,4);
	glBindTexture(GL_TEXTURE_2D, 0); 
	
	if (depth_was_on) glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

Shader &DrawBuffers::setShaderAndVBOs(bool stencil) {
	Shader &shader = stencil ? shader_stencil : shader_depth;
	shader.use();
	int loc_pos = 0;
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glVertexAttribPointer(loc_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(loc_pos);
	if (not stencil) {
		loc_pos = 1;
		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glVertexAttribPointer(loc_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(loc_pos);
	}
	return shader;
}

	
void DrawBuffers::addImGuiSettings (GLFWwindow * window) {
	ImGui::Combo("Buffer (B)", &buffer_id, vbuffers);
	if (buffer_id==1) {
		int v = getStencilValueUnderMouseCursor(window);
		ImGui::Text("Value under mouse: %i",v);
	} else if (buffer_id==2)
		ImGui::SliderFloat("Exp",&depht_exp,0.1f, 10.f);	
}

void DrawBuffers::draw (int w, int h) {
	if (buffer_id==1)drawStencil(256);
	else if (buffer_id==2) drawDepth(w,h,depht_exp);
}

void DrawBuffers::setNextBuffer ( ) {
	buffer_id = (buffer_id+1)%vbuffers.size();
}

