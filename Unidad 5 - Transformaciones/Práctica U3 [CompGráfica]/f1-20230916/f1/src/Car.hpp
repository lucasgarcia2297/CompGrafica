#ifndef CAR_HPP
#define CAR_HPP
#include "Track.hpp"

using Angulo = float; // los �ngulos est�n en radianes

struct Car {
	float x = 0, y = 0, ang = 0; // posicion y orientacion en la pistaz
	float vel = 0; // velocidad actual
	Angulo rang1 = 0; // direccion de las ruedas delanteras respecto al auto (eje x del mouse) 
	Angulo rang2 = 0; // giro de las ruedas sobre su eje, cuando el auto avanza
	const float top_speed = 50; // velociad m�xima
	
	Car() = default;
	Car(float x0, float y0, float ang0) : x(x0), y(y0), ang(ang0) { }
	
	void Move(const Track &track, float acel, float dir, bool analog=false); // funci�n que aplica la "f�sica" y actualiza el estado
};

#endif
