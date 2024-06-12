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

#define VERSION 20220919

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo");
	setCommonCallbacks(window);
	
	// load shaders
	Shader shader("shaders/PBR");
	
	// model
	std::vector<std::string> models_names = { "suzanne", "sphere", "teapot", "chookity" };
	int current_model = 0, loaded_model = -1;
	Model model;
	// material
	glm::vec3 diffuse_color = { 0.8, 0.2, 0.1 };
	glm::vec3 base_reflectivity = { 1.0, 0.7, 0.1 };
	float metallic = 0.9;
	float roughness = 0.4;
	// lights
	bool enable_light_1 = true;
	glm::vec4 light_pos_1 = {-2.f,2.f,2.f,0.f};
	glm::vec3 light_color_1 = {.7f,.7f,.7f};
	float ambient_strength_1 = 0.15;
	bool enable_light_2 = true;
	glm::vec4 light_pos_2 = {+2.f,2.f,2.f,0.f};
	glm::vec3 light_color_2 = {.7f,.7f,.7f};
	float ambient_strength_2 = 0.15;
	// misc
	bool rotate = true;
	view_target.x = -0.40;
	glm::vec3 background_color = {0.3f,0.3f,0.4f};
	glm::vec3 black = {0.f,0.f,0.f};
	FrameTimer ftime;
	glEnable(GL_FRAMEBUFFER_SRGB); 
	// main loop
	do {
		
		glClearColor(background_color.r,background_color.g,background_color.b,1.f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		// reload model if necessary
		if (loaded_model!=current_model) { 
			model = Model::loadSingle(models_names[current_model],Model::fNoTextures);
			loaded_model = current_model;
		}
		
		// auto-rotate
		double dt = ftime.newFrame();
		if (rotate) model_angle += static_cast<float>(0.5f*dt);
		
		// select a shader
		shader.use();
		setMatrixes(shader);
		
		// setup light and material
		shader.setLightX(1,light_pos_1,light_color_1,ambient_strength_1);
		shader.setLightX(2,light_pos_2,light_color_2,ambient_strength_2);
		shader.setUniform("roughness", roughness);
		shader.setUniform("metallic", metallic);
		shader.setUniform("diffuseColor", diffuse_color);
		shader.setUniform("baseReflectivity", base_reflectivity);
		
		// send geometry
		shader.setBuffers(model.buffers);
		model.buffers.draw();
		
		// settings sub-window
		window.ImGuiDialog(nullptr,[&](){
			ImGui::SetNextWindowPos({10,10},ImGuiCond_FirstUseEver);
			ImGui::Begin("CG Example");
			ImGui::Combo(".obj", &current_model,models_names);		
			ImGui::ColorEdit3("Background",&background_color.r);
			ImGui::Checkbox("Auto-rotate",&rotate);
			ImGui::Separator();
			if (ImGui::TreeNode("Light 1")) {
				ImGui::Checkbox("Enable",&enable_light_1);
				ImGui::InputFloat4("Position",&light_pos_1.r);
				ImGui::ColorEdit3("Color",&light_color_1.r);
				ImGui::SliderFloat("Ambient",&ambient_strength_1,0.f,1.f,"%.2f");
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light 2")) {
				ImGui::Checkbox("Enable",&enable_light_2);
				ImGui::InputFloat4("Position",&light_pos_2.r);
				ImGui::ColorEdit3("Color",&light_color_2.r);
				ImGui::SliderFloat("Ambient",&ambient_strength_2,0.f,1.f,"%.2f");
				ImGui::TreePop();
			}
			ImGui::Separator();
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Material")) {
				ImGui::ColorEdit3("Diffuse Color",&diffuse_color.r);
				ImGui::ColorEdit3("Base Reflectivity",&base_reflectivity.r);
				ImGui::SliderFloat("Roughness",&roughness,0.f,1.f,"%.2f");
				ImGui::SliderFloat("Metallic",&metallic,0.f,1.f,"%.2f");
				ImGui::TreePop();
			}
			ImGui::End();
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}
