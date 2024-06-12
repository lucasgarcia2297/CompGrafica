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
#include "SubDivMesh.hpp"
#include "SubDivMeshRenderer.hpp"

#define VERSION 20221013

// models and settings
std::vector<std::string> models_names = { "cubo", "icosahedron", "plano", "suzanne", "star" };
int current_model = 0;
bool fill = true, nodes = true, wireframe = true, smooth = false, 
	 reload_mesh = true, mesh_modified = false;

// extraa callbacks
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

SubDivMesh mesh;
void subdivide(SubDivMesh &mesh);

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo",true);
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	view_fov = 60.f;
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); 
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.8f,0.8f,0.9f,1.f);
	Shader shader_flat("shaders/flat"),
	       shader_smooth("shaders/smooth"),
		   shader_wireframe("shaders/wireframe");
	SubDivMeshRenderer renderer;
	
	// main loop
	Material material;
	material.ka = material.kd = glm::vec3{.8f,.4f,.4f};
	material.ks = glm::vec3{.5f,.5f,.5f};
	material.shininess = 50.f;
	
	FrameTimer timer;
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		if (reload_mesh) {
			mesh = SubDivMesh("models/"+models_names[current_model]+".dat");
			reload_mesh = false; mesh_modified = true;
		}
		if (mesh_modified) {
			renderer = makeRenderer(mesh);
			mesh_modified = false;
		}
		
		if (nodes) {
			shader_wireframe.use();
			setMatrixes(shader_wireframe);
			renderer.drawPoints(shader_wireframe);
		}
		
		if (wireframe) {
			shader_wireframe.use();
			setMatrixes(shader_wireframe);
			renderer.drawLines(shader_wireframe);
		}
		
		if (fill) {
			Shader &shader = smooth ? shader_smooth : shader_flat;
			shader.use();
			setMatrixes(shader);
			shader.setLight(glm::vec4{2.f,1.f,5.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.25f);
			shader.setMaterial(material);
			renderer.drawTriangles(shader);
		}
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			if (ImGui::Combo(".dat (O)", &current_model,models_names)) reload_mesh = true;
			ImGui::Checkbox("Fill (F)",&fill);
			ImGui::Checkbox("Wireframe (W)",&wireframe);
			ImGui::Checkbox("Nodes (N)",&nodes);
			ImGui::Checkbox("Smooth Shading (S)",&smooth);
			if (ImGui::Button("Subdivide (D)")) { subdivide(mesh); mesh_modified = true; }
			if (ImGui::Button("Reset (R)")) reload_mesh = true;
			ImGui::Text("Nodes: %i, Elements: %i",mesh.n.size(),mesh.e.size());
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'D': subdivide(mesh); mesh_modified = true; break;
		case 'F': fill = !fill; break;
		case 'N': nodes = !nodes; break;
		case 'W': wireframe = !wireframe; break;
		case 'S': smooth = !smooth; break;
		case 'R': reload_mesh=true; break;
		case 'O': case 'M': current_model = (current_model+1)%models_names.size(); reload_mesh = true; break;
		}
	}
}

// La struct Arista guarda los dos indices de nodos de una arista
// Siempre pone primero el menor indice, para facilitar la búsqueda en lista ordenada;
//    es para usar con el Mapa de más abajo, para asociar un nodo nuevo a una arista vieja
struct Arista {
	int n[2];
	Arista(int n1, int n2) {
		n[0]=n1; n[1]=n2;
		if (n[0]>n[1]) std::swap(n[0],n[1]);
	}
	Arista(Elemento &e, int i) { // i-esima arista de un elemento
		n[0]=e[i]; n[1]=e[i+1];
		if (n[0]>n[1]) std::swap(n[0],n[1]); // pierde el orden del elemento
	}
	const bool operator<(const Arista &a) const {
		return (n[0]<a.n[0]||(n[0]==a.n[0]&&n[1]<a.n[1]));
	}
};

// Mapa sirve para guardar una asociación entre una arista y un indice de nodo (que no es de la arista)
using Mapa = std::map<Arista,int>;

void subdivide(SubDivMesh &mesh) {
	Mapa mapa;
	/// @@@@@: Implementar Catmull-Clark... lineamientos:
	//  Los nodos originales estan en las posiciones 0 a #n-1 de m.n,
	//  Los elementos orignales estan en las posiciones 0 a #e-1 de m.e
	///  1) Por cada elemento, agregar el centroide (nuevos nodos: #n a #n+#e-1)
	int e_size, n_size,vert;
	//por cada iteracion de la subdivision:
	e_size = mesh.e.size();//cantidad de elementos  e inicial
	n_size = mesh.n.size();//cantidad de nodos  n inicial
	
	for(int i=0; i<e_size; i++){//por cada Elemento en e
		glm::vec3 centroide(0.f,0.f,0.f);
		vert = mesh.e[i].nv; //numero de vertices del elemento (3 o 4)
		for(int j=0; j<vert; j++){
			centroide += mesh.n[mesh.e[i].n[j]].p; //posicion del nodo e[i].n[j]
		}
		centroide /= vert; //promedio
		mesh.n.push_back(Nodo(centroide)); //agregar centroide a los n nodos
	}
	//mesh.makeVecinos();
	///  2) Por cada arista de cada cara, agregar un pto en el medio que es
	//      promedio de los vertices de la arista y los centroides de las caras 
	//      adyacentes. Aca hay que usar los elementos vecinos.
	
	///esto se puede hacer porque hay un orden entre las caras (un sentido en el que se recorre los vertices que forman las aristas)
	Arista a(-1,-1);
//	glm::vec3 promedio(0.f,0.f,0.f);
	Nodo promedio({0.f,0.f,0.f});
	for(int i=0; i<e_size; i++){//por cada Elemento en e
		vert = mesh.e[i].nv; //numero de vertices del elemento (3 o 4)
		for(int j=0; j<vert; j++){ //por cada vertice del elemento
			if(j == vert-1) { //si esta parado en el ultimo 
				a = Arista(mesh.e[i].n[j], mesh.e[i].n[0]);//hace el ultimo con el primero
			}else {
				a = Arista(mesh.e[i].n[j], mesh.e[i].n[j+1]);
			}
			if(mapa.find(a) == mapa.end()){ //si no ecuentra la arista (esto es para no repetirla)
				int veci_i = mesh.e[i].v[j]; // indice de la cara vecina
				if(veci_i != -1){ //si tiene vecino
					glm::vec3 centr_actual = mesh.n[n_size+i].p; // centroide del elemento actual
					glm::vec3 centr_vecino = mesh.n[n_size+veci_i].p; //centroide del elemento vecino 
					promedio = Nodo((centr_actual + centr_vecino + mesh.n[a.n[0]].p + mesh.n[a.n[1]].p) * 0.25f); //promedio con los centroides y los puntos de la arista
				}else{ //no tiene vecinos
					promedio = Nodo((mesh.n[a.n[0]].p + mesh.n[a.n[1]].p ) * 0.5f); //promedio sobre la arista
				}
				mesh.n.push_back(promedio);//agregar el nuevo Nodo promedio
				mapa[a] = (mesh.n.size()-1); //indice del Nodo agregado asociado a la Arista a;
				
			}
		}
	}
	
	///  3) Armar los elementos nuevos
	//      Los quads se dividen en 4, (uno reemplaza al original, los otros 3 se agregan)
	//      Los triangulos se dividen en 3, (uno reemplaza al original, los otros 2 se agregan)
	//      Para encontrar los nodos de las aristas usar el mapa que armaron en el paso 2
	//      Ordenar los nodos de todos los elementos nuevos con un mismo criterio (por ej, 
	//      siempre poner primero al centroide del elemento), para simplificar el paso 4.
	Arista ar1(-1,-1), ar2(-1,-1);
	for(int i=0; i<e_size; i++){ //por cada Elemento en e
		vert = mesh.e[i].nv; //numero de vertices del elemento (3 o 4)
		for(int j=0; j<vert ; j++){ //por cada vertice en el Elemento e[i]
			if(j == 0){ //en el primer Nodo del Elemento
				ar1 = Arista(mesh.e[i],vert-1); // ultima arista
				ar2 = Arista(mesh.e[i],j); //arista 0
			}else {
				ar1 = Arista(mesh.e[i],j-1); //arista j-1
				ar2 = Arista(mesh.e[i],j);//arista j
			}
			int i_centroide = n_size+i; //indice del centrodide del Elemento e
			int i1 = mapa[ar1]; //indice del Nodo de la arista ar1
			int i2 = mapa[ar2]; //indice del Nodo de la arista ar2
			if(j == vert-1) {mesh.reemplazarElemento(i,i_centroide,i1,mesh.e[i].n[j],i2);}//reemplaza el nuevo Elemento al vector e
			else {mesh.agregarElemento(i_centroide,i1,mesh.e[i].n[j],i2);} //agrego el nuevos al vector e
		}
		
	}
	mesh.makeVecinos();
	
	///  4) Calcular las nuevas posiciones de los nodos originales
	glm::vec3 p,f,r, n_pos;
	float c;
	for(int i =0; i<n_size; i++){ // Por cada Nodo en n
		r = {0.f,0.f,0.f};
		float c = 0.f;
		p = mesh.n[i].p; //posicion del Nodo actual
		
		if(mesh.n[i].es_frontera){ //para Nodos de borde
			for(int j=0; j<n_size; j++){ //todos los Nodos contra el actual
				a = Arista(i,j);
				if(mapa.find(a) != mapa.end()){ //si hay una arista con estos nodos
					int indice = mapa[a]; //usamos el indice para buscar la posicion del Nodo
					if( mesh.n[indice].es_frontera) {
						c++; //incrementamos contador de nodo
						r += mesh.n[indice].p; //sumamos las posiciones
					}
				}
			}
			r /= c; //el promedio
			mesh.n[i].p = (r + p) /2.f; //calculamos la nueva posicion del Nodo n[i];
		} else { //para nodos interiores
			c= 0;
			f = {0.f,0.f,0.f};
			for(int j=0; j<n_size; j++){ //todos los Nodos contra el actual
				a = Arista(i,j);
				if(mapa.find(a) != mapa.end()){ //si hay una arista con estos nodos
					c++; //incrementamos contador de nodo
					int indice = mapa[a]; //usamos el indice para buscar la posicion del Nodo
					r += mesh.n[indice].p; //sumamos las posiciones
				}
			}
			r /= c; //el promedio
			c =0;
			float c_e = mesh.n[i].e.size();
			for(int z=0; z< c_e; z++){ //por cada Elemento formado por el Nodo n[i]
				int nv = mesh.e[mesh.n[i].e[z]].nv; //numero de vertices del Elemento n[i].e[z]
				for(int j=0; j<nv;j++){
					int pos_nueva = mesh.e[mesh.n[i].e[z]].n[j]; //posicion del Nodo que forma el Elemento n.[i]e[z]
					if(pos_nueva >= n_size && pos_nueva < n_size+e_size){
						c++;
						f += mesh.n[pos_nueva].p; //se suman las posiciones de los centroides de las caras 
						break;
					}
				}
			}
			f /= c; //promedio
			mesh.n[i].p = ( r * 4.f - f + p * (c_e - 3.f))/ c_e; //calculamos la nueva posicion del Nodo n[i];
		}
	}
	
}
