////////////////////////////////////////////////////////////////////////
//////////////////   PUNTO 2D DE COORDENADAS ENTERAS   /////////////////
////////////////////////////////////////////////////////////////////////

#ifndef _P2E_ // para no incluir dos veces
#define _P2E_

#include <cmath> // abs
#include <cstddef> // size_t
#include "utiles.h" // cuadrado

class p2e{
public:
  union{
    int coord[2];
    struct{int x, y;};
  };
  // constructores
  p2e(){x=y=0;}
  p2e(int xi, int yi) {x=xi;y=yi;}
  p2e(const p2e& p) {x=p.x;y=p.y;}
  p2e(const p2e &px, const p2e &py)
    {x=px.x;y=py.y;}
  p2e(const int* p){x=p[0];y=p[1];}

  // cordenadas
  int &operator[](int i){return coord[i];}
  int operator[](int i) const {return coord[i];}

  //referencia
  operator const int*() const {return coord;}

  // copia
  p2e& operator=(const int* p){x=p[0];y=p[1];return *this;}
  p2e& operator=(const p2e &p){x=p.x;y=p.y;return *this;}
  void set_x(const p2e &p){x=p.x;}
  void set_y(const p2e &p){y=p.y;}
  void zero(){x=y=0;}

  // comparación
  bool eq_b2(const int*,const long long r2=0) const; // circulo dado r^2
  bool eq_b2(const p2e& p,const long long r2=0) const {return eq_b2(p.coord,r2);}
  bool eq_c(const int*,const int r=0) const; // cuadrado
  bool eq_c(const p2e& p,const int r=0) const {return eq_c(p.coord,r);}
  bool operator ==(const int* p) const 
    {return (x==p[0]&&y==p[1]);}
  bool operator ==(const p2e& p) const {return *this==p.coord;}
  bool operator !=(const int* p) const 
    {return (x!=p[0]||y!=p[1]);}
  bool operator !=(const p2e& p) const {return *this!=p.coord;}

  //compara y asigna componente a componente (bounding box)
  void set_min_max(p2e& pmin,p2e& pmax) const;

    // operadores y operaciones
  p2e& operator+=(const p2e& p)  // +=
    {x+=p.x;y+=p.y;return *this;}
  p2e& operator-=(const p2e& p)  // -=
    {x-=p.x;y-=p.y;return *this;}
  p2e& operator*=(int t)           // *=
  {x*=t;y*=t;return *this;} /////////////////////////////OJOOO HABIA UN BUG SERIO
  p2e& operator/=(int t)           // /=
    {x/=t;y/=t;return *this;}

  p2e operator-() const              // opuesto
    {return p2e(-x,-y);}
  p2e operator+(const p2e &p) const // suma
    {return p2e(x+p.x,y+p.y);}
  p2e operator-(const p2e &p) const // resta
    {return p2e(x-p.x,y-p.y);}
  long long operator%(const p2e &p) const // producto vectorial (% =precedencia *)
    {return x*p.y-y*p.x;}
  long long operator*(const p2e &p) const // producto escalar
    {return x*p.x+y*p.y;}
  p2e operator*(int t) const // escalar * p2e y viceversa
    {return p2e(t*x,t*y);}
  friend p2e operator*(int t,const p2e &p) {return (p*t);}
  p2e operator/(int t) const // cociente por un escalar
    {return p2e(x/t,y/t);}

  // distancia
  long long distancia2(const p2e &p) const // distancia al cuadrado
    {return cuadrado<long long>(x-p.x)+cuadrado<long long>(y-p.y);}
  friend long long distancia2(const p2e &a,const p2e &b)
    {return a.distancia2(b);}
  int distanciac(const p2e &p) const // maximo delta x o y
    {return maximo(abs(x-p.x),abs(y-p.y));}
  friend int distanciac(const p2e &a, const p2e &b)
    {return a.distanciac(b);}

  // modulo
  long long mod2() const   // modulo al cuadrado
    {return cuadrado<long long>(x)+cuadrado<long long>(y);}
  friend long long mod2(const p2e &p) {return p.mod2();}
  int modc() const   // maxima componente (abs)
    {return maximo(abs(x),abs(y));}
  friend int modc(const p2e &p) {return p.modc();}

  p2e giro90() const // gira la 90 grados a la izq
    {return p2e(-y,x);}
  friend p2e giro90(const p2e &p) {return p.giro90();}

    //interseccion
  bool interseccion( // la pone en *this
      const p2e &p00, const p2e &p01, 
      const p2e &p10, const p2e &p11, 
      double params[2]); // dos parametros double
  bool interseccion( // la pone en *this
      const p2e &p00, const p2e &p01, 
      const p2e &p10, const p2e &p11, 
      long long params[3]); // los parametros reales son 0/2 y 1/2
  friend bool hay_interseccion( // solo testea si hay interseccion en los segmentos
      const p2e &p00, const p2e &p01, 
      const p2e &p10, const p2e &p11);
  friend bool hay_interseccion_estricta( // si hay interseccion interior
      const p2e &p00, const p2e &p01, 
      const p2e &p10, const p2e &p11);
 
  // circunferencia de 3 puntos 
  // a=4*area; c=(centro-p0)*a;
  friend void c3(const p2e &, const p2e &, const p2e &, long long *c, long long &a);
  friend void c3 (const p2e **p,long long *c, long long &a)
    {c3(*(p[0]),*(p[1]),*(p[2]),c,a);}

  // funciones de forma o coordenadas baricentricas
  // -------
  // calcula un punto segun sus dos coordenadas (constructor)
  p2e(const p2e &p0, const p2e &p1, const float &f)
    {x=redondea(p0.x*(1-f)+p1.x*f); y=redondea(p0.y*(1-f)+p1.y*f);}
  // calcula un punto segun sus tres coordenadas (constructor)
  // las tres areas parciales y la total (areas por 4)
  p2e(const p2e &p0, const p2e &p1, const p2e &p2, const long long *a);
  // calcula las tres coordenadas de area
  // si el area ya viene precalculada poner true
  void fforma(const p2e&, const p2e&, const p2e&, long long *, bool area_calculada=false) const;
};

// constantes globales
static const p2e pzero(0,0);
static const p2e _ex(1,0),_ey(0,1);
static const size_t SZp2e=sizeof(p2e);

#endif
