#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"
#include "Car.hpp"
#include "Render.hpp"

#define VERSION 20230916

// models and settings
bool wireframe = false, play = false, top_view = true, use_helmet = true;

// extra callbacks (atajos de teclado para cambiar de modo y camara)
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

// función para sensar joystick o teclado (si no hay joystick) y definir las 
// entradas para el control del auto
std::tuple<float,float,bool> getInput(GLFWwindow *window);

// game logic data
Track track("mapa.png",100,100);
Car car(+66,-35,1.38);
double accum_dt, lap_time, best_lap;
std::vector<double> laps_history;

void resetSimulation() {
	// game logic data
	car.x = 66;
	car.y = -35;
	car.ang = 1.38;
	car.vel = 0.0;
	accum_dt = lap_time = 0.0;
	best_lap = 999.999;
	laps_history.clear();	
}


// main: crea la ventana, carga los modelos e implementa el bucle principal
int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo",true);
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.4f,0.4f,0.8f,1.f);
	Shader shader_phong("shaders/phong");
	
	// car parts models
	std::vector<Part> parts; parts.reserve(8);
	parts.push_back({"axis",      true,Model::load("axis",      Model::fDontFit)});
	parts.push_back({"body",      true,Model::load("body",      Model::fDontFit)});
	parts.push_back({"wheels",    true,Model::load("wheel",     Model::fDontFit)});
	parts.push_back({"front wing",true,Model::load("front_wing",Model::fDontFit)});
	parts.push_back({"rear wing", true,Model::load("rear_wing", Model::fDontFit)});
	parts.push_back({"driver",    true,Model::load("driver",    Model::fDontFit)});
	parts.push_back({"chookity",  true,Model::load("chookity",  Model::fDontFit)});
	
	// main loop
	resetSimulation();
	FrameTimer ftime;
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		// actualizar las pos del auto y de la camara
		double elapsed_time = ftime.newFrame();
		accum_dt += elapsed_time;
		if (play) lap_time += elapsed_time;
		auto in = getInput(window);
		while (accum_dt>1.0/60.0) { 
			car.Move(track,std::get<0>(in),std::get<1>(in),std::get<2>(in));
			if (track.isFinishLine(car.x,car.y) and lap_time>15) {
				laps_history.push_back(lap_time); lap_time = 0.0;
				best_lap = std::min(best_lap,laps_history.back());
			}
			accum_dt-=1.0/60.0;
			setViewAndProjectionMatrixes(car);
		}
		
		// setear matrices y renderizar
		if (play) {
			renderTrack();
			renderShadow(car,parts);
		}
		renderCar(car,parts,shader_phong);
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			ImGui::Checkbox("Play (P)",&play);
			if (play) {
				ImGui::Checkbox("Top View (T)",&top_view);
				ImGui::Separator();
				ImGui::LabelText("","Current Lap Time: %.3f s", lap_time);
				ImGui::LabelText("","Best Lap: %.3f s", best_lap);
				if (ImGui::TreeNode("Lap times history")) {
					int n = laps_history.size();
					for (int i = std::max(0,n-10); i<n; ++i)
						ImGui::LabelText("","Lap %i: %.3f s", i+1,laps_history[i]);
					ImGui::TreePop();
				}
				if (ImGui::Button("Reset (R)")) resetSimulation();
			} else {
				ImGui::Checkbox("Wireframe (W)",&wireframe);
				ImGui::Separator();
				if (ImGui::TreeNode("Parts")) {
					for(Part &p : parts)
						if (&p!=&parts.back())
							ImGui::Checkbox(p.name.c_str(),&p.show);
					ImGui::TreePop();
				}
			}
			ImGui::Separator();
			if (ImGui::TreeNode("car")) {
				ImGui::LabelText("","x: %f",car.x);
				ImGui::LabelText("","y: %f",car.y);
				ImGui::LabelText("","ang: %f",car.ang);
				ImGui::LabelText("","rang1: %f",car.rang1);
				ImGui::LabelText("","rang2: %f",car.rang2);
				ImGui::TreePop();
			}
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'W': wireframe = !wireframe; break;
		case 'T': if (!play) play = true; else top_view = !top_view; break;
		case 'R': resetSimulation(); break;
		case 'P': play = !play; break;
		case 'C': case 'H': use_helmet = !use_helmet; break;
		}
	}
}

std::tuple<float,float,bool> getInput(GLFWwindow *window) {
	float acel = 0.f, dir = 0.f; bool analog = false;
	if (glfwGetKey(window,GLFW_KEY_UP)   ==GLFW_PRESS) acel += 1.f;
	if (glfwGetKey(window,GLFW_KEY_DOWN) ==GLFW_PRESS) acel -= 1.f;
	if (glfwGetKey(window,GLFW_KEY_RIGHT)==GLFW_PRESS) dir += 1.f;
	if (glfwGetKey(window,GLFW_KEY_LEFT) ==GLFW_PRESS) dir -= 1.f;
	
	int count;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
	if (count) { dir = axes[0]; acel = -axes[1]; analog = true; }
	return std::make_tuple(acel,dir,analog);
}
