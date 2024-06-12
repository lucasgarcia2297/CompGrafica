#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "utils.hpp"
#include "Debug.hpp"
#include <iostream>
using namespace std;

BoundingBox::BoundingBox(glm::vec3 &p1, glm::vec3 &p2) 
	: pmin({std::min(p1.x,p2.x),std::min(p1.y,p2.y),std::min(p1.z,p2.z)}),
      pmax({std::max(p1.x,p2.x),std::max(p1.y,p2.y),std::max(p1.z,p2.z)}) 
{
	
}
	
bool BoundingBox::contiene(glm::vec3 &p) const {
	return p.x>=pmin.x && p.x<=pmax.x &&
		p.y>=pmin.y && p.y<=pmax.y &&
		p.z>=pmin.z && p.z<=pmax.z;
}

Pesos calcularPesos(glm::vec3 x0, glm::vec3 x1, glm::vec3 x2, glm::vec3 &x) {
	/*3 puntos, es decir que me muevo sobre el plano*/
	/// @todo: implementar
	
	float alfa0, alfa1, alfa2;	//
	glm::vec3 a0,a1,a2,at; 		//Defino las variables para calcular el area de cada una y el area total.
//	at = glm::cross(x2-x1,x0-x1);
//	a0 = glm::cross(x2-x1,x-x1);
//	a1 = glm::cross(x0-x2,x-x2);
//	a2 = glm::cross(x-x1,x0-x1);
	at = glm::cross(x1-x0,x2-x0);
	a0 = glm::cross(x2-x1,x-x1);
	a1 = glm::cross(x0-x2,x-x2);
	a2 = glm::cross(x1-x0,x-x0);
	
	
	alfa0 = glm::dot(a0,at)/(glm::dot(at,at));
	alfa1 = glm::dot(a1,at)/(glm::dot(at,at));
	alfa2 = glm::dot(a2,at)/(glm::dot(at,at));
	
//cout << sqrt(pow(at[0],2)+pow(at[1],2)+pow(at[2],2));
//cout <<" "<< alfa0 << " " << alfa1 << " " << alfa2 << " suma: "<<alfa0+alfa1+alfa2 << endl;
//	cg_error("debe implementar la funcion calcularPesos (utils.cpp)");
	return {alfa0,alfa1,alfa2};
}
