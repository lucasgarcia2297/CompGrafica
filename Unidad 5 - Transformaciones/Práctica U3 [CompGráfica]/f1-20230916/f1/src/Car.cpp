#include <cmath>
#include "Car.hpp"

constexpr float PI = 3.14159265359f;
constexpr float G2R = PI/180.f;

void Car::Move(const Track &track, float acel, float dir, bool analog) {
	// frenar si se sale de la pista
	if ((not track.isAsphalt(x,y)) && vel>top_speed/4) {
		acel = -1; dir /= 2;
		static bool chang_dir = true;
		if ((chang_dir=!chang_dir)) ang += (rand()%101-50)/35000.f*vel;
	}
	// aplicar los controles
	double target_dir = dir*0.006*float(10*vel+50*(top_speed-vel))/top_speed;
	if (analog) rang1 = dir<0 ? std::max<double>(dir,2*target_dir) : std::min<double>(dir,2*target_dir);
	else        rang1 = std::fabs(target_dir)<std::fabs(rang1) ? (2*rang1+target_dir)/3 : (9*rang1+target_dir)/10;
	if (!acel) acel = -.03;
	// mover el auto
	vel += acel*.60-.10;
	if (vel<0) vel=0;
	else if (vel>top_speed) vel=top_speed;
	x += vel*std::cos(ang)/100;
	y += vel*std::sin(ang)/100;
	// la pista es ciclica
	if (x<-track.Width()) x += track.Width()*2;
	else if (x>track.Width()) x -= track.Width()*2;
	if (y<-track.Height()) y += track.Height()*2;
	else if (y>track.Height()) y -= track.Height()*2;
	ang += rang1*vel/150;
	rang2 += vel/10;
}


