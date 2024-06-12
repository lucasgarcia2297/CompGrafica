#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "ObjMesh.hpp"
#include "Shaders.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Model.hpp"

#define VERSION 20230925
#include <cmath>
#include <glm/trigonometric.hpp>
using namespace std;

Shader *shader_coords_ptr = nullptr;
bool show_wireframe = false, show_coords = false, shader_ok = false;

std::vector<glm::vec2> generateTextureCoordinatesForBottle(const std::vector<glm::vec3> &v) {
	/// @todo: generar el vector de coordenadas de texturas para los vertices de la botella
	
	std::vector<glm::vec2> cord_text(v.size());
	for(int i =0; i<v.size(); i++){ //por cada vetice de la malla
		float s = atan2(v[i].x, v[i].z); //el angulo con el que barre la circunferencia con valores positivo
		s /= (3.14f); // variacion en [0,1]
		float t = (-v[i].y*2)+0.5f; // variacion de altura
		
		cord_text[i] = glm::vec2(s,t);
	}
	return cord_text;
}

std::vector<glm::vec2> generateTextureCoordinatesForLid(const std::vector<glm::vec3> &v) {
	/// @todo: generar el vector de coordenadas de texturas para los vertices de la tapa
	std::vector<glm::vec2> cord_text(v.size());
	for(int i =0; i<v.size(); i++){
		float s = (v[i].x+0.10) *5.f;
		float t = (v[i].z+0.10) *5.f;
		cord_text[i] = glm::vec2(s,t);
	}
	return cord_text;
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

bool reload_shader_coords();

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Texturas");
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
	glEnable(GL_BLEND); glad_glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.6f,0.6f,0.8f,1.f);
	Shader shader_texture("shaders/texture");
	Shader shader_coords("shaders/texture");
	Shader shader_lines("shaders/lines");
	shader_coords_ptr = &shader_coords;
	reload_shader_coords();
	
	// load model and assign texture
	auto models = Model::load("models/bottle",Model::fKeepGeometry|Model::fTextureDontFlipV);
	Model &bottle = models[0], &lid = models[1];
	bottle.buffers.updateTexCoords(generateTextureCoordinatesForBottle(bottle.geometry.positions),true);
	bottle.texture = Texture("models/label.png", Texture::fClampT|Texture::fY0OnTop);
	lid.buffers.updateTexCoords(generateTextureCoordinatesForLid(lid.geometry.positions),true);
	lid.texture = Texture("models/lid.png", Texture::fClampT|Texture::fClampS|Texture::fY0OnTop);
	
	// main loop
	glPolygonOffset(-1.f,1.f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		for(int j=0;j<2;++j) {
			Shader &shader = j ? shader_lines : ( show_coords ? shader_coords : shader_texture );
			glPolygonMode(GL_FRONT_AND_BACK,j?GL_LINE:GL_FILL);
			shader.use();
			setMatrixes(shader);
			shader.setLight(glm::vec4{1.f,-1.f,5.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.15f);
			for(Model &mod : models) {
				mod.texture.bind();
				shader.setMaterial(mod.material);
				shader.setBuffers(mod.buffers);
				mod.buffers.draw();
			}
			if (not show_wireframe) break;
		}
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			ImGui::Checkbox("Show wireframe (W)",&show_wireframe);
			ImGui::Checkbox("Use shader coords(C)",&show_coords);
			if (ImGui::Button("Reload shader coords (F5)")) reload_shader_coords();
			if (!shader_ok) ImGui::Text("   Error compiling shader coords");
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
	 
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'W': show_wireframe = !show_wireframe; break;
		case 'C': show_coords = !show_coords; break;
		case GLFW_KEY_F5: reload_shader_coords(); break;
		}
	}
}

bool reload_shader_coords() {
	try {
		Shader new_shader("shaders/coords");
		*shader_coords_ptr = std::move(new_shader);
		return (shader_ok = true);
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return (shader_ok = false);
	}
}
