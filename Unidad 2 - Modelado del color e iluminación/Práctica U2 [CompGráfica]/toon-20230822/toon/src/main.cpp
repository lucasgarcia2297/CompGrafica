#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <tuple>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"

#define VERSION 20230522

Shader *shader_toon_ptr = nullptr;
bool rotate = true, enable_toon = false;

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

bool reload_shader_toon();

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width, win_height, "CG Demo");
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	
	// load shader_toons
	Shader shader_phong("shaders/phong");
	Shader shader_lines("shaders/lines");
	Shader shader_toon("shaders/phong"); // phong, por si toon no compila
	shader_toon_ptr = &shader_toon;
	bool shader_ok = reload_shader_toon();
	
	// model
	auto models = Model::load("homer", Model::fNoTextures);
	// material
	float mat_shininess = 100, mat_spec_strength = 0.5f;
	// lights
	glm::vec4 light_pos = {-2.f,2.f,2.f,0.f};
	glm::vec3 light_color = {1.f,1.f,1.f};
	float ambient_strength = 0.25f;
	// outline
	float outline_factor = 5.f;
	// misc
	view_target.x = -0.40;
	glm::vec3 black = {0.f, 0.f, 0.f};
	glClearColor(0.9f, 0.9f, 0.7f, 1.f);
	FrameTimer ftime;
	
	// main loop
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		// auto-rotate
		double dt = ftime.newFrame();
		if (rotate) model_angle += static_cast<float>(0.5f*dt);
		
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		
		// lines
		if (enable_toon) {
			shader_lines.use();
			setMatrixes(shader_lines);
			shader_lines.setUniform("outline_factor", outline_factor);
			glCullFace(GL_FRONT);
			for(auto &model : models) {
				if (glm::dot(model.material.kd, model.material.kd)<0.1f)
					continue; // no "engrosar" los pelos de homero
				shader_lines.setBuffers(model.buffers);
				model.buffers.draw();
			}
		}
		
		// interior
		Shader &shader = enable_toon ? shader_toon : shader_phong;
		shader.use();
		setMatrixes(shader);
		shader.setLight(light_pos,light_color,ambient_strength);
		shader.setUniform("shininess",mat_shininess);
		shader.setUniform("specularStrength",mat_spec_strength);
		shader.setUniform("cameraPosition",view_pos);
		glCullFace(GL_BACK);
		for(Model &model : models) {
			shader.setBuffers(model.buffers);
			shader.setUniform("objectColor", model.material.kd*1.75f);
			model.buffers.draw();
		}
		
		// settings sub-window
		window.ImGuiDialog(nullptr,[&](){
			ImGui::SetNextWindowPos({10,10}, ImGuiCond_FirstUseEver);
			ImGui::Begin("Toon Shading");
			ImGui::Checkbox("Auto-rotate (R)", &rotate);
			ImGui::Checkbox("Toon mode (T)", &enable_toon);
			
			ImGui::SliderFloat("Specular Strength", &mat_spec_strength, 0.f, 1.f, "%.2f");
			ImGui::SliderFloat("Shininess", &mat_shininess, 1.f,256.f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Outline", &outline_factor, 0.f,50.f, "%.2f");
			
			if (ImGui::Button("Reload shader_toon (F5)")) reload_shader_toon();
			
			ImGui::Text(shader_ok?"   Shader compilation: Ok":"    Shader compilation: ERROR");
			ImGui::End();
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
			case 'R': rotate = !rotate; break;
			case 'T': enable_toon = !enable_toon; break;
			case GLFW_KEY_F5: reload_shader_toon(); break;
		}
	}
}

bool reload_shader_toon() {
	try {
		Shader new_shader("shaders/toon");
		*shader_toon_ptr = std::move(new_shader);
		return true;
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
}
