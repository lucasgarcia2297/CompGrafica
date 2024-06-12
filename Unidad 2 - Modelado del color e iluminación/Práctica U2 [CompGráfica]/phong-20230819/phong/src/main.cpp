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
#include "MatLib.hpp"

#define VERSION 20220919

auto tie(const Material &m1) { // func auxiliar para comparar materiales
	return std::tie(m1.ka,m1.kd,m1.ks,m1.ke,m1.shininess,m1.opacity,m1.texture);
}

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo");
	setCommonCallbacks(window);
	
	// load shaders
	Shader shader("shaders/phong");
	
	// model
	std::vector<std::string> models_names = { "suzanne", "sphere", "teapot" };
	int current_model = 0, loaded_model = -1;
	Model model;
	// material
	matlib::init();
	Material material;
	int curr_material = 0;
	// lights
	bool enable_light_1 = true;
	glm::vec4 light_pos_1 = {-2.f,2.f,2.f,0.f};
	glm::vec3 light_color_1 = {.7f,.7f,.7f};
	float ambient_strength_1 = 0.25f;
	bool enable_light_2 = false;
	glm::vec4 light_pos_2 = {+2.f,2.f,2.f,0.f};
	glm::vec3 light_color_2 = {.7f,.7f,.7f};
	float ambient_strength_2 = 0.25f;
	// misc
	bool rotate = true;
	view_target.x = -0.40;
	glm::vec3 background_color = {0.3f,0.3f,0.4f};
	glm::vec3 black = {0.f,0.f,0.f};
	FrameTimer ftime;
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
		shader.setLightX(1,light_pos_1,enable_light_1?light_color_1:black,ambient_strength_1);
		shader.setLightX(2,light_pos_2,enable_light_2?light_color_2:black,ambient_strength_2);
		shader.setMaterial(material);
		
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
				ImGui::SliderFloat("Ambient",&ambient_strength_1,0.f,1.f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light 2")) {
				ImGui::Checkbox("Enable",&enable_light_2);
				ImGui::InputFloat4("Position",&light_pos_2.r);
				ImGui::ColorEdit3("Color",&light_color_2.r);
				ImGui::SliderFloat("Ambient",&ambient_strength_2,0.f,1.f);
				ImGui::TreePop();
			}
			ImGui::Separator();
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Material")) {
				ImGui::ColorEdit3("Ambient",&material.ka.r);
				ImGui::ColorEdit3("Diffuse",&material.kd.r);
				ImGui::ColorEdit3("Specular",&material.ks.r);
				ImGui::SliderFloat("Shininess",&material.shininess,1.f,255.f,"%.2f",ImGuiSliderFlags_Logarithmic);
				ImGui::ColorEdit3("Emission",&material.ke.r);
				if (curr_material and tie(material)!=tie(matlib::materials[curr_material])) curr_material = 0;
				if (ImGui::Combo("Presets",&curr_material,matlib::names))
					material = matlib::materials[curr_material];
				ImGui::TreePop();
			}
			ImGui::End();
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}
