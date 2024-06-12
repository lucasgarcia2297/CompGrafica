#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <tuple>
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
#include "Debug.hpp"

#define VERSION 20221019

Shader *shaderHolo_ptr= nullptr;

bool reload_shader_holo() {
	try {
		Shader new_shader("shaders/hologram");
		*shaderHolo_ptr= std::move(new_shader);
		return true;
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
}
// texture array
std::vector<std::string> texture_vec;
static int current_tex = 0;

void reload_textures(Model &model)
{
	model.texture02 = Texture(texture_vec[current_tex],true,true);
};


int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Texturas");
	setCommonCallbacks(window);
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
	glEnable(GL_BLEND); glad_glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.2f,0.2f,0.2f,1.f);
	
	bool wireframe = false;
	
	// Scene Objects
	Shader shaderTex("shaders/texture");
	auto modelHoloDeck = Model::loadSingle("holoDeck", 0);
	modelHoloDeck.texture01 = Texture("models/holoDeck.png",true,true);
	std::cout<< shaderTex.getProgramId() << std::endl;
	
	Shader shaderCon("shaders/cone");
	auto modelHoloCone = Model::loadSingle("holoCone", 0);
	modelHoloCone.texture01 = Texture("models/noise03.png",true,true);
	std::cout<< shaderCon.getProgramId()<< std::endl;
	
	// Chookity
	Shader shaderHolo("shaders/hologram");
	shaderHolo_ptr = &shaderHolo;
	bool shader_ok = reload_shader_holo();
	std::cout<< shaderHolo.getProgramId() << std::endl;
	
	std::cout<<"shader ok: " << shader_ok << std::endl;

	// load model and assign texture
	auto model = Model::loadSingle("chookity", 0);
	model.texture01 = Texture("models/chookity.png",true,true);
	model.texture02 = Texture("models/noise03.png",true,true);
	
	// SHADER USER DATA
	texture_vec.push_back("models/noise01.png");
	texture_vec.push_back("models/noise02.png");
	texture_vec.push_back("models/noise03.png");
	
	// HOLO SHADER PARAMETERS
	float holo_freq = 1.0f;
	float holo_tresh = 0.0f;
	float holo_speed = 1.0f;
	float test04 = 0.5f;
	float uv_scale[2] = {1.0f, 1.0f};
	float uv_scroll[2] = {0.0f, 0.0f};
	float time = 0.0f;
	float color01[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	
	// used to send time to the shader
	const clock_t begin_time = clock();
	
	do {
		time = float( clock () - begin_time )/CLOCKS_PER_SEC;
		Shader &shader = shaderHolo;
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		// ----------- DRAW CHOOKITY ----------
		shader.use();
		setMatrixes(shader);
		shader.setLight(glm::vec4{2.f,-2.f,-4.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.15f);
		
		shader.setUniform("cookity_tex", 0);
		model.texture01.bind(0);
		shader.setUniform("noise_tex", 1);
		model.texture02.bind(1);
		
		shader.setMaterial(model.material);
		shader.setBuffers(model.buffers);
		
		if(shader_ok)
		{
			shader.setUniform("freq", holo_freq);
			shader.setUniform("tresh", holo_tresh);
			shader.setUniform("sine_speed", holo_speed);
			shader.setUniform("time", time);
			shader.setUniform("color01", glm::vec4(color01[0], color01[1], color01[2], color01[3]));
			shader.setUniform("uv_scale", glm::vec2(uv_scale[0], uv_scale[1]));;
			shader.setUniform("uv_scroll", glm::vec2(uv_scroll[0], uv_scroll[1]));;
		}

		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		model.buffers.draw();
		
		// ----------- DRAW SCENE OBJECTS ----------
		
		shaderTex.use();
		setMatrixes(shaderTex);
		shaderTex.setLight(glm::vec4{2.f,-2.f,-4.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.15f);
		modelHoloDeck.texture01.bind();
		shaderTex.setMaterial(modelHoloDeck.material);
		shaderTex.setBuffers(modelHoloDeck.buffers);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		modelHoloDeck.buffers.draw();
		
		shaderCon.use();
		shaderCon.setUniform("time", time);
		shaderCon.setUniform("color01", glm::vec4(color01[0], color01[1], color01[2], color01[3]));
		setMatrixes(shaderCon);
		modelHoloCone.texture01.bind();
		shaderCon.setBuffers(modelHoloCone.buffers);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		modelHoloCone.buffers.draw();
		
		// ----------- UI -------------------
		window.ImGuiDialog(nullptr,[&](){
			ImGui::GetIO().FontGlobalScale = 1.2f;
			ImGui::SetNextWindowPos({10,10}, ImGuiCond_FirstUseEver);
			ImGui::Begin("Inner Sphere");
			ImGui::SliderFloat("Holo Frequency", &holo_freq, 0.f,10.f, "%.2f");
			ImGui::SliderFloat("Holo Treshold", &holo_tresh, 0.f,1.f, "%.2f");
			ImGui::SliderFloat("Holo Speed", &holo_speed, 0.f,10.f, "%.2f");
			ImGui::DragFloat2("Uv Scale 01", uv_scale, 1.0f, 0.0f, 5.0f, "%.2f");
			ImGui::DragFloat2("Uv Scroll 01", uv_scroll, 1.0f, 0.0f, 10.0f, "%.2f");
			ImGui::ColorEdit4("Color01", color01);
			if(ImGui::Combo("Noise Texture", &current_tex, texture_vec)) reload_textures(model);
			
			// Define new UI for shader Uniforms here ...
			
			if (ImGui::Button("Reload shader_toon (F5)")) reload_shader_holo();
			
			ImGui::Text(shader_ok?"   Shader compilation: Ok":"    Shader compilation: ERROR");
			ImGui::End();
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}


