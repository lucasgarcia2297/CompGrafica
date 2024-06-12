#ifndef ALGEBRA_HPP
#define ALGEBRA_HPP
#include <cmath>

struct Punto {
  float x,y,z; ///< coordenadas del punto
  explicit Punto() : x(0), y(0), z(0) { }
  explicit Punto(float _x, float _y, float _z=0) : x(_x), y(_y), z(_z) { }
};

struct Vector {
  float x,y,z; ///< componentes del vector
  explicit Vector() : x(0), y(0), z(0) { }
  explicit Vector(float _x, float _y, float _z=0) : x(_x), y(_y), z(_z) { }
  /// modulo al cuadrado
  float Mod2() const { return x*x+y*y+z*z; } 
  /// modulo
  float Mod() const { return std::sqrt(Mod2()); }
};

/// opuesto de un vector
inline Vector operator-(Vector v) { return Vector(-v.x,-v.y,-v.z); }

/// punto + vector = punto
inline Punto operator+(Punto p, Vector v) { return Punto(p.x+v.x,p.y+v.y,p.z+v.z); }
inline Punto& operator+=(Punto &p, Vector v) { return p=p+v; }
/// punto - vector = punto
inline Punto operator-(Punto p, Vector v) { return p+(-v); }
inline Punto& operator-=(Punto &p, Vector v) { return p=p-v; }

/// vector + vector = vector
inline Vector operator+(Vector v1, Vector v2) { return Vector(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z); }
inline Vector& operator+=(Vector &v1, Vector v2) { return v1=v1+v2; }
/// vector - vector = vector
inline Vector operator-(Vector v1, Vector v2) { return Vector(v1.x-v2.x,v1.y-v2.y,v1.z-v2.z); }
inline Vector& operator-=(Vector &v1, Vector v2) { return v1=v1-v2; }

/// punto - punto = vector
inline Vector operator-(Punto p1, Punto p2) { return Vector(p1.x-p2.x,p1.y-p2.y,p1.z-p2.z); }

/// vector * escalar = vector
inline Vector operator*(Vector v, float s) { return Vector(v.x*s,v.y*s,v.z*s); }
inline Vector& operator*=(Vector &v, float s) { return v=v*s; }
/// escalar * vector = vector
inline Vector operator*(float s, Vector v) { return Vector(v.x*s,v.y*s,v.z*s); }

/// vector / escalar = vector
inline Vector operator/(Vector v, float s) { return Vector(v.x/s,v.y/s,v.z/s); }
inline Vector& operator/=(Vector &v, float s) { return v=v/s; }

/// producto escalar/punto
inline float dot_product(Vector v1, Vector v2) { return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z; }
/// producto escalar/punto
inline float operator*(Vector v1, Vector v2) { return dot_product(v1,v2); }

/// producto vectorial/cruz
inline Vector cross_product(Vector v1, Vector v2) { 
  return Vector(v1.y*v2.z-v1.z*v2.y,
                v1.z*v2.x-v1.x*v2.z,
                v1.x*v2.y-v1.y*v2.x); 
}
/// producto vectorial/cruz
inline Vector operator%(Vector v1, Vector v2) { return cross_product(v1,v2); }

/// normalización de un vector
inline Vector norm(Vector v) { return v/v.Mod(); }

/// interpolación lineal/afin entre 2 o 3 puntos

inline Punto interp2(Punto p1, Punto p2, float alpha1 = .5f) { return p1 + alpha1*(p2-p1); }

inline Punto interp3(Punto p1, Punto p2, Punto p3, float alpha1=1.f/3.f, float alpha2=1.f/3.f) { 
	float a3 = 1 -alpha1-alpha2;
	return Punto(alpha1*p1.x+alpha2*p2.x+a3*p3.x,
							 alpha1*p1.y+alpha2*p2.y+a3*p3.y,
							 alpha1*p1.z+alpha2*p2.z+a3*p3.z);
}

#endif
