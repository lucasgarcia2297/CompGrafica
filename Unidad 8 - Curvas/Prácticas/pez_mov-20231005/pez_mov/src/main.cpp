#include <algorithm>
#include <stdexcept>
#include <vector>
#include <limits>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"
#include "BezierRenderer.hpp"
#include "Spline.hpp"

#define VERSION 20231005
using namespace std;

// settings
bool show_axis = false, show_fish = false, show_spline = true, show_poly = true, animate = true;

// curva
static const int degree = 3;
int ctrl_pt = -1, cant_pts = 6; // pto de control seleccionado (para el evento de arrastre)
Spline spline( { {-1.f,0.f,0.f}, {0.f,0.f,-1.f}, {1.f,0.f,0.f}, {0.f,0.f,1.f} }, true );

float modulo(const glm::vec3 &V){
	return sqrt(pow(V[0],2)+pow(V[1],2)+pow(V[2],2));
}
void updateControlPointsAround(Spline &spline, int ctrl_pt) {
	/// @todo: actualizar los puntos anterior y posterior a ctrl_pt
	///con catmull-Rom
//		int pto_anterior = ctrl_pt-1;  //indice del punto pi-
//		int pto_siguiente = ctrl_pt+1; //indice del punto pi+
//		
//		glm::vec3 p_actual = spline.getControlPoint(ctrl_pt);		//punto interpolado actual (pi)
//		glm::vec3 p_anterior = spline.getControlPoint(ctrl_pt-3);  //punto interpolado anterior (pi-1)
//		glm::vec3 p_siguiente = spline.getControlPoint(ctrl_pt+3); //punto interpolado siguiente (pi+1)
//		
//		glm::vec3 vi= (p_siguiente-p_anterior)/2.f;  //derivada en el punto interpolado actual
//		
//		glm::vec3 pi_anterior = p_actual-(vi/3.f); 	//punto de control anterior al actual (pi-)
//		glm::vec3 pi_siguiente = p_actual+(vi/3.f); //punto de control siguiente al actual (pi+)
//		
//		
//		spline.setControlPoint(pto_anterior,pi_anterior); 	//actualización del punto (pi-)
//		spline.setControlPoint(pto_siguiente,pi_siguiente); //actualización del punto (pi+)
//	
//		
	///con el método de Overhausser
	int pto_anterior = ctrl_pt-1; 
	int pto_siguiente = ctrl_pt+1;
	
	glm::vec3 p_actual = spline.getControlPoint(ctrl_pt);		//punto interpolado actual
	glm::vec3 p_anterior = spline.getControlPoint(ctrl_pt-3);  //punto interpolado anterior
	glm::vec3 p_siguiente = spline.getControlPoint(ctrl_pt+3); //punto interpolado siguiente
	
	
	glm::vec3 vi_anterior = (p_actual-p_anterior)/(modulo(p_actual-p_anterior));  //derivada en el punto anterior
	glm::vec3 vi_siguiente = (p_siguiente-p_actual)/(modulo(p_siguiente-p_actual));  //derivada en el punto  siguiente
	float m = min(glm::distance(p_actual,p_anterior),glm::distance(p_actual,p_siguiente));
	glm::vec3 vi = (modulo(p_siguiente-p_actual)*vi_anterior + modulo(p_actual-p_anterior)*vi_siguiente )/(modulo(p_actual-p_anterior) +modulo(p_siguiente-p_actual)) ;  //derivada en el punto interpolado actual
	
	glm::vec3 pi_anterior = p_actual-m*vi/3.f; 	//punto de control anterior al actual
	glm::vec3 pi_siguiente = p_actual+m*vi/3.f; 	//punto de control anterior al actual
	
	spline.setControlPoint(pto_anterior,pi_anterior);	
	spline.setControlPoint(pto_siguiente,pi_siguiente);
//	//	
}

// callbacks
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);
void characterCallback(GLFWwindow* glfw_win, unsigned int code);

glm::mat4 getTransform(const Spline &spline, double t) {
	/// @todo: obtener los ejes y la nueva posicion del origen en funcion
	///        de la curva y el valor del parametro t
	glm::vec3 deriv;
	glm::vec3 pos = spline.at(t,deriv); //Obtengo la posición y la velocidad (vector derivada) del pez, en el tiempo t.
	deriv = normalize(deriv);
	
	//	glm::vec3 e_x(1.f,0.f,0.f);
	//	glm::vec3 e_y(0.f,1.f,0.f);
	//	glm::vec3 e_z(0.f,0.f,1.f);
	//	glm::vec3 pos(0.f,0.f,0.f);
	
//	float angulo = atan2(deriv.z,deriv.x); //Ángulo formado entre el vector derivada y el eje +x
	float esc = 0.2;  //escalado del pescadito
//	
//	//Matriz de transformación.
//	glm::vec3 e_x(cos(angulo)*esc, 0.0f, sin(angulo)*esc);
//	glm::vec3 e_y(0.f,esc,0.f);
//	glm::vec3 e_z(-sin(angulo)*esc, 0.0f, cos(angulo)*esc);
	
	// armar la matriz
//	glm::mat4 m(1.f);
//	for(int k=0;k<3;++k) { 
//		m[0][k] = e_x[k];
//		m[1][k] = e_y[k];
//		m[2][k] = e_z[k];
//		m[3][k] = pos[k];
//	}
//	glm::mat4 m(glm::vec4(e_x,0.f),glm::vec4(e_y,0.f),glm::vec4(e_z,0.f),glm::vec4(pos,1.f));
	
	glm::vec3 e_y(0.f,1.f,0.f);
	glm::mat4 m(glm::vec4(deriv,0.f)*esc,glm::vec4(e_y,0.f)*esc,glm::vec4(cross(deriv,e_y)*esc,0.f),glm::vec4(pos,1.f));
	
	return m;
}

// cuando cambia la cant de tramos, regenerar la spline
void remapSpline(Spline &spline, int cant_pts) {
	if (cant_pts<3) return;
	if (static_cast<int>(spline.getPieces().size()) == cant_pts) return;
	std::vector<glm::vec3> vp;
	double dt = 1.0/cant_pts;
	for(int i=0;i<cant_pts;++i)
		vp.push_back(spline.at(i*dt));
	spline = Spline(vp,true);
	for(int i=0;i<spline.getControlPointsCount();i+=degree) 
		updateControlPointsAround(spline,i);
}

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo",true);
	glfwSetFramebufferSizeCallback(window, common_callbacks::viewResizeCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCharCallback(window, characterCallback);
	
	// setup OpenGL state and load the model
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glClearColor(0.4f,0.4f,0.7f,1.f);
	Shader shader_fish("shaders/fish");
	Shader shader_phong("shaders/phong");
	auto fish = Model::load("models/fish");
	auto axis = Model::load("models/axis",Model::fDontFit);
	BezierRenderer bezier_renderer(500);
	model_angle = .33; view_angle = .85;
	
	glm::vec4 light_pos = {2.f,2.f,4.f,0.f};
	
	// main loop
	FrameTimer ftime;
	float t = 0.f, speed = .05f;
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		remapSpline(spline,cant_pts);
		
		// draw models and curve
		float dt = ftime.newFrame();
		if (animate) {
			t += dt*speed; while (t>1.f) t-=1.f; 
		}
		if (show_fish) {
			shader_fish.use();
			shader_fish.setLight(light_pos, glm::vec3{1.f,1.f,1.f}, 0.15f);
			shader_fish.setUniform("time",t*20);
			glm::mat4 m = getTransform(spline, t);
			auto mats = common_callbacks::getMatrixes();
			for(Model &model : fish) {
				shader_fish.setMatrixes(mats[0]*m,mats[1],mats[2]);
				shader_fish.setBuffers(model.buffers);
				shader_fish.setMaterial(model.material);
				model.buffers.draw();
			}
		}
		
		if (show_spline or show_poly) {
			setMatrixes(bezier_renderer.getShader());
			for(const auto & curve : spline.getPieces()) {
				bezier_renderer.update(curve);
				glPointSize(1);
				if (show_spline) bezier_renderer.drawCurve();
				glPointSize(5);
				if (show_poly) bezier_renderer.drawPoly();
			}
		}
		
		if (show_axis) {
			shader_phong.use();
			shader_phong.setLight(light_pos, glm::vec3{1.f,1.f,1.f}, 0.15f);
			setMatrixes(shader_phong);
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			for(const Model &model : axis) {
				shader_phong.setBuffers(model.buffers);
				shader_phong.setMaterial(model.material);
				model.buffers.draw();
			}
		}
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			ImGui::Checkbox("Pez (P)",&show_fish);
			ImGui::Checkbox("Spline (S)",&show_spline);
			ImGui::Checkbox("Pol. Ctrl. (C)",&show_poly);
			ImGui::Checkbox("Ejes (J)",&show_axis);
			ImGui::Checkbox("Animar (A)",&animate);
			ImGui::SliderFloat("Velocidad",&speed,0.005f,0.5f);
			ImGui::SliderFloat("T",&t,0.f,1.f);
			if (ImGui::InputInt("Cant. Pts.",&cant_pts,1,1))
				if (cant_pts<3) cant_pts=3;
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void characterCallback(GLFWwindow* glfw_win, unsigned int code) {
	switch (code) {
		case 'a': case 'A':animate = !animate; break;
		case 's': case 'S':show_spline = !show_spline; break;
		case 'p': case 'P':show_fish = !show_fish; break;
		case 'j': case 'J':show_axis = !show_axis; break;
		case 'c': case 'C':show_poly = !show_poly; break;
		case '+': ++cant_pts; break;
		case '-': --cant_pts; break;
	}
}

glm::vec3 viewportToPlane(double xpos, double ypos) {
	auto ms = common_callbacks::getMatrixes(); // { model, view, projection }
	auto inv_matrix = glm::inverse(ms[2]*ms[1]*ms[0]); // ndc->world
	auto pa = inv_matrix * glm::vec4{ float(xpos)/win_width*2.f-1.f,
		(1.f-float(ypos)/win_height)*2.f-1.f,
		0.f, 1.f }; // point on near
	auto pb = inv_matrix * glm::vec4{ float(xpos)/win_width*2.f-1.f,
		(1.f-float(ypos)/win_height)*2.f-1.f,
		1.f, 1.f }; // point on far
	float alpha = pa[1]/(pa[1]-pb[1]);
	auto p = pa*(1-alpha) + pb*alpha; // point on plane
	return {p[0]/p[3],0.f,p[2]/p[3]};
}

void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	if (ctrl_pt==-1) common_callbacks::mouseMoveCallback(window,xpos,ypos);
	else {
		spline.setControlPoint(ctrl_pt,viewportToPlane(xpos,ypos));
		if (ctrl_pt%degree==0) {
			updateControlPointsAround(spline,ctrl_pt);
			updateControlPointsAround(spline,ctrl_pt+degree);
			updateControlPointsAround(spline,ctrl_pt-degree);
		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) return;
	ctrl_pt = -1;
	if (action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		auto p = viewportToPlane(xpos,ypos);
		float dmin = .1f;
		for(int i=0;i<spline.getControlPointsCount(); ++i) {
			double aux = glm::distance(p,spline.getControlPoint(i));
			if (aux < dmin) { dmin = aux; ctrl_pt = i; }
		}
	}
	if (ctrl_pt==-1) common_callbacks::mouseButtonCallback(window,button,action,mods);
}

