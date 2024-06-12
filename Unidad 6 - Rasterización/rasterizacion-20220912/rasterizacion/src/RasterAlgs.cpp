#include <forward_list>
#include <iostream>
#include <GL/gl.h>
#include <cmath>
#include "RasterAlgs.hpp"
using namespace std;

void drawSegment(paintPixelFunction paintPixel, glm::vec2 p0, glm::vec2 p1) {
	/// @todo: implementar algun algoritmo de rasterizacion de segmentos 
	//DDA 
	float dx, dy;
	dx = p1.x - p0.x;
	dy = p1.y - p0.y;
	
	if(abs(dx) >= abs(dy)){
		// tendencia horizontal o iguales
		float x, y, x1;
		x = round(p0.x);
		y = round(p0.y);
		x1 = round(p1.x);
		paintPixel(glm::vec2 {x,y});//se pinta el primer punto
		if(x == x1) return; // caso donde el punto p0 en x sea igual que p1 en x
		
		float m = dy/dx;
		if(dx<0){ //swapeo de los puntos 
			x = round(p1.x);
			x1 = round(p0.x);
			y = p1.y;
		}
		while(++x < x1){
			paintPixel(glm::vec2 {x,round(y)});//pinto cada pixel
			y+= m; //incremento de la pendiente (Constante)
			//			x+= 1;
		}
		
	}else{
		// tendencia vertical
		float x, y, y1;
		x = p0.x;
		y = round(p0.y);
		y1 = round(p1.y);
		float m = dx/dy;
		
		if(dy<0){ //swapeo de los puntos 
			y = round(p1.y);
			y1 = round(p0.y);
			x = p1.x;
		}
		while(y < y1){
			paintPixel(glm::vec2 {round(x),y});//pinto cada pixel
			x+= m; //incremento de la pendiente (Constante)
			y++;
		}
	}
//	
//	float dx = (p1.x - p0.x);
//	float dy = (p1.y - p0.y);
//	
//	if(abs(dx)>=abs(dy)){
//		if(dx<0){
////			glm::vec2 p = p0;
//			std::swap(p0,p1);
////			dx = -dx;
////			dy = -dy;
//		}
//		if(dx == 0 ){
//			cout << "divide por 0"<<endl;
//			paintPixel(p0);
//			return;
//		}else{
//			float m = dy/dx;
//			float x, y;
//			x = round(p0.x);
//			y = p0.y;
//			while(x <p1.x ){
//				paintPixel(glm::vec2(x,round(y)));
//				y += m;
//				x++;
//			}
//		}
//	}else{
//		if(dy<0){
//			std::swap(p0,p1);
////			dy = -dy;
////			dx = -dx;
//		}
//		float m = dx/dy;
//		float x, y;
//		x = p0.x;
//		y = round(p0.y);
//		if(dy == 0 ){
//			cout << "divide por 0"<<endl;
//			paintPixel(p0);
//			return;
//		}else{
//		while(y < p1.y){
//			paintPixel(glm::vec2 {round(x),y});
//			x += m;
//			y++;
//			}
//		}
//	}
}
	
	


void drawCurve(paintPixelFunction paintPixel, curveEvalFunction evalCurve) {
//	/// @todo: implementar algun algoritmo de rasterizacion de curvas
//	float t = 0;
//	float dt = 0;
//	while(t <= 1){
//		glm::vec2 p = evalCurve(t).p;
//		glm::vec2 d = evalCurve(t).d;
//		glm::vec2 p_a;
//		paintPixel(glm::vec2 {round(p.x),round(p.y)});
////		dt = min(1/fabs(d.x),1/fabs(d.y));
//		if(fabs(d.x) > fabs(d.y)){
//			dt = 1/fabs(d.x);
////			t += dt;
////			p_a = evalCurve(t).p;
//		}else{
//			dt = 1/fabs(d.y);
////			t += dt;
////			p_a = evalCurve(t).p;
//		}
//		p_a = evalCurve(t).p;
//		while(abs(round(p_a.y) - round(p.y)) >= 2 or abs(round(p_a.x) - round(p.x)) >= 2){
//			dt = dt/2.f;
//		}
//		t+=dt;
//		
//		if(round(p.x) )
//	}
}






