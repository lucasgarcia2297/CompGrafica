#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"
#include "DrawBuffers.hpp"

#define VERSION 20221125

// window, models and settings
Window window;
DrawBuffers draw_buffers;
Model model;
Texture alternative_texture;
struct Instance {
	glm::mat4 matrix;
	glm::vec3 color_var;
	float rot_speed;
	bool is_the_choosen_one;
};
std::vector<Instance> instances(256);
int selected_instance = -1;
double time_to_find_the_one;
Shader shader_texture, shader_silhouette;
float angle_object = 0.f, outline_width  = 0.125f;
int level = 1;
const std::vector<std::string> vlevels = { "Easy", "Medium", "Hard" };

void drawInstance(const Instance &instance, Shader &shader);

// extra callbacks
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

void initInstances();

void outlineInstance(const Instance &instance, glm::vec3 color_silhouette, float time) {
	shader_silhouette.use();
	shader_silhouette.setUniform("outline_width",outline_width);
	glEnable(GL_STENCIL_TEST);
	shader_silhouette.setUniform("color",glm::vec4{color_silhouette,0.4f});
	shader_silhouette.setUniform("time",time);

	///Dibujo del pollo.
	glStencilFunc(GL_NEVER,1,~0);
	glStencilOp(GL_REPLACE,GL_KEEP,GL_KEEP);
	drawInstance(instance,shader_texture);
	
	///Config. del Z-BUFFER para considerar los z mayores.
	glDepthFunc(GL_GEQUAL);
	///Dibujo de la silueta.
	///Como estoy comparando z mayores, solo se dibuja en la parte "tapada".
	glStencilFunc(GL_NOTEQUAL,1,~0);
	///Entender bien esta config. 
	glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
	drawInstance(instance,shader_silhouette);
	///Invertimos la configuración del Z. Consideramos z menores.
	glDepthFunc(GL_LESS);
	
	
	///Dibujo de la silueta, solo en en las partes visibles.
	glStencilFunc(GL_EQUAL,0,~0);
//	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	shader_silhouette.setUniform("color",glm::vec4{color_silhouette,1.f});
	drawInstance(instance,shader_silhouette);
	///Desabilitamos stencil test.
	glDisable(GL_STENCIL_TEST);	
}

int main() {
	
	// initialize window and setup callbacks
	window = Window(win_width,win_height,"Outlining",Window::fDefaults|Window::fBlend);
	setCommonCallbacks(window);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	
	// setup OpenGL state and load shaders
	shader_silhouette = Shader ("shaders/silhouette");
	shader_texture = Shader ("shaders/texture");
	
	// load model and init instances
	model = Model::loadSingle("chookity");
	alternative_texture = Texture("models/choosen.png",0);
	initInstances();
	
	// main loop
	FrameTimer ftime;
	view_target.y = -.5f;
//	view_pos.z *= 1.5;
	view_angle = .35;
	std::cout << "Where, where, where, where is chookity?" << std::endl;
	do {
		glClearColor(0.8f,0.8f,0.7f,1.f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		
		view_angle = std::min(std::max(view_angle,0.01f),1.72f);
		
		// auto-rotate
		double dt = ftime.newFrame();
		if (selected_instance==-1 or (not instances[selected_instance].is_the_choosen_one))
			time_to_find_the_one += dt;
		angle_object += static_cast<float>(1.f*dt*level);
		
		for(auto &inst: instances)
			drawInstance(inst,shader_texture);
		
		if (selected_instance!=-1) {
			float s = instances[selected_instance].is_the_choosen_one ? 1.f : 0.f;
			outlineInstance(instances[selected_instance],{1.f-s,s,0.f}, time_to_find_the_one);
		}
		
		draw_buffers.draw(win_width,win_height);
		
		// settings sub-window
		window.ImGuiDialog("Find The Choosen One!",[&](){
			ImGui::LabelText("","Time: %.3f s",time_to_find_the_one);
			ImGui::Combo("Level (L)", &level, vlevels);
			if (ImGui::Button("Restart (R)")) initInstances();
			ImGui::SliderFloat("outline width",&outline_width,.05,.5);
			draw_buffers.addImGuiSettings(window);
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'R': initInstances(); break;
		case 'L': level = (level+1)%vlevels.size(); break;
		case 'B': draw_buffers.setNextBuffer(); break;
		}
	}
}

int findSelection(int x, int y) { //llamo a la función con las coordenadas x e y del punto en la pantalla
	
	glDisable(GL_MULTISAMPLE);
	glClearColor(1.f,1.f,1.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	glm::mat4 mrot = glm::rotate(glm::mat4{1.f},angle_object,{0.f,1.f,0.f});
	shader_silhouette.use();
	shader_silhouette.setUniform("outline_width",0.f);
	glm::vec3 color_silhouette(1.f,0.f,0.f);
	for(size_t i=0;i<instances.size();++i) { 
		const auto &mat = instances[i].matrix;
		float r = (i%256)/255.f;
		float g = ((i/256)%256)/255.f;
		float b = ((i/256/256)%256)/255.f;
		shader_silhouette.setUniform("color",glm::vec4{r,g,b,1.f});
		drawInstance(instances[i],shader_silhouette);
	}
	glFlush();
	glFinish();
	glReadBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	unsigned char data[3];
	glReadPixels(x,y,1,1, GL_RGB, GL_UNSIGNED_BYTE, data);
	glEnable(GL_MULTISAMPLE);
	int sel = int(data[0])+int(data[1])*256+int(data[2])*256*256;
	if (sel>=instances.size()) sel = -1;
	return sel;
}

bool isDoubleClick(int button, int action) {
	if (button!=GLFW_MOUSE_BUTTON_LEFT) return false;
	if(action!=GLFW_RELEASE) return false;
	static double prev = 0.0;
	double cur = glfwGetTime();
	bool ret = (cur-prev<.5);
	prev = cur;
	return ret;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	common_callbacks::mouseButtonCallback(window,button,action,mods);
	if (isDoubleClick(button,action)) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		selected_instance = findSelection(int(xpos),win_height-int(ypos));
		if (selected_instance!=-1 and (not instances[selected_instance].is_the_choosen_one))
			time_to_find_the_one += level+1;
	}
}

void drawInstance(const Instance &instance, Shader &shader) {
	shader.use();
	
	auto mats = common_callbacks::getMatrixes();
	glm::mat4 mrot = glm::rotate(glm::mat4{1.f},angle_object*instance.rot_speed,{0.f,1.f,0.f});
	shader.setMatrixes(mats[0]*instance.matrix*mrot,mats[1],mats[2]);
	
	// setup light and material
	shader.setLight({-5.0,5.f,5.f,1.f}, glm::vec3{1.f,1.f,1.f}, 0.4f);
	shader.setMaterial(model.material);
	shader.setUniform("colorVar",instance.color_var);
	(instance.is_the_choosen_one ? alternative_texture : model.texture).bind();
	
	// send geometry
	shader.setBuffers(model.buffers);
	model.buffers.draw();
}

void initInstances() {
	time_to_find_the_one = 0.0;
	selected_instance = -1;
	std::srand(std::time(0));
	int choosen_one = rand()%instances.size()*.7;
	for(size_t i=0;i<instances.size();++i) {
		Instance &inst = instances[i];
		auto rndf = [](float max) {
			static std::mt19937 mt(std::time(0));
			static std::uniform_real_distribution<float> rd;
			return -max+rd(mt)*2.f*max;
		};
		constexpr float PI = 3.14159265359;
		constexpr float GR = 1.61803398875;
		inst.is_the_choosen_one = i == choosen_one;
		auto mat = glm::mat4{1.f};
		float dist = 0.15f+3.f*std::pow(float(i)/instances.size(),0.6f), ang = i*2*PI*GR;
		mat = glm::translate(mat ,glm::vec3{std::sin(ang),0,std::cos(ang)}*dist);
		mat = glm::scale(mat ,glm::vec3{1.f+rndf(.1f),1.f+rndf(.1f),1.f+rndf(.1f)}*.25f);
		mat = glm::rotate(mat ,rndf(3.14f),{rndf(.20f),1.f,rndf(.20f)});
		inst.matrix = mat ;
		inst.rot_speed = 1.2f+rndf(.6f); 
		if (rand()%2) inst.rot_speed *= -1.f;
		inst.color_var = {rndf(.1f),rndf(.1f),rndf(.1f)};
	}
}
