////////////////////////////////////////////////////////////////////////
//////////////////   PUNTO 2D DE COORDENADAS ENTERAS   /////////////////
////////////////////////////////////////////////////////////////////////
#include "p2e.h"

  // comparación
bool p2e::eq_b2(const int* p,const long long r2) const { // circulo dado r^2
  long long d2=cuadrado<long long>(p[0]-x); if (d2>r2) return false;
  d2+=cuadrado<long long>(p[1]-y); if (d2>r2) return false;
  return true;
}
bool p2e::eq_c(const int* p,const int r) const { // cuadrado
  if (abs(p[0]-x)>r||abs(p[1]-y)>r) return false;
  return true;
}

  //compara y asigna componente a componente (bounding box)
void p2e::set_min_max(p2e& pmin,p2e& pmax) const {
  if (x<pmin[0]) pmin[0]=x;
  if (y<pmin[1]) pmin[1]=y;
  if (x>pmax[0]) pmax[0]=x;
  if (y>pmax[1]) pmax[1]=y;
}

  //interseccion
bool p2e::interseccion(
    const p2e &p00, const p2e &p01, 
    const p2e &p10, const p2e &p11, 
    double params[2]){
  p2e d0(p01-p00),d1(p11-p10),d01=p10-p00;
  double det=d1%d0;
  static const double epsilon=1e-16;
  if (fabs(det)<epsilon) {zero(); return false;}
  params[0]=(d1%d01)/det;
  params[1]=(d0%d01)/det;
  x=redondea(p00[0]+params[0]*d0[0]);
  y=redondea(p00[1]+params[1]*d0[1]);
  return true;
}
bool p2e::interseccion(
    const p2e &p00, const p2e &p01, 
    const p2e &p10, const p2e &p11, 
    long long params[3]){
  static const double epsilon=1e-16;
  p2e d0(p01-p00),d1(p11-p10),d01=p10-p00;
  params[2]=d1%d0;
  if (fabs(params[2])<epsilon) {zero(); return false;}
  params[0]=d1%d01;
  params[1]=d0%d01;
  x=redondea(p00[0]+(double(params[0])/params[2])*d0[0]);
  y=redondea(p00[1]+(double(params[1])/params[2])*d0[1]);
  return true;
}
bool hay_interseccion(
    const p2e &p00, const p2e &p01, 
    const p2e &p10, const p2e &p11){
  p2e d0(p01-p00),d1(p11-p10),d01=p10-p00;
  long long det=d1%d0;
  if (!det) return false;
  long long t=d1%d01; 
  if (det>0){
    if (t<0||t>det) return false;
  }
  else{
    if (t>0||t<det) return false;
  }
  t=d1%d01;
  if (det>0){
    if (t<0||t>det) return false;
  }
  else{
    if (t>0||t<det) return false;
  }
  return true;
}
bool hay_interseccion_estricta(
    const p2e &p00, const p2e &p01, 
    const p2e &p10, const p2e &p11){
  p2e d0(p01-p00),d1(p11-p10),d01=p10-p00;
  long long det=d1%d0;
  if (!det) return false;
  long long t=d1%d01; 
  if (det>0){
    if (t<=0||t>=det) return false;
  }
  else{
    if (t>=0||t<=det) return false;
  }
  t=d1%d01;
  if (det>0){
    if (t<=0||t>=det) return false;
  }
  else{
    if (t>=0||t<=det) return false;
  }
  return true;
}

  // circunferencia de tres puntos
  // Pi=pi-p0; i=1,2
  // (Pi-C)^2 = r^2 = C^2 = Pi^2-2Pi*C+C^2 => 2*Pi*C=Pi^2
  // a es el cuadruple del area del triangulo
  // c es (centro-p0)*a
void c3(const p2e &p0, const p2e &p1, const p2e &p2, long long *c, long long &a){
  p2e P1=p1-p0,P2=p2-p0; // origen en p0
  a=(P1%P2)<<1;
  long long m2P1=P1.mod2(),m2P2=P2.mod2();
  c[0]=m2P1*P2[1]-m2P2*P1[1];
  c[1]=P1[0]*m2P2-P2[0]*m2P1;
}

  // funciones de forma o coordenadas de area
  // a tiene las tres areas parciales y la total (areas por 4)
  // calcula un punto segun sus tres coordenadas
p2e::p2e(const p2e &p0, const p2e &p1, const p2e &p2, const long long *a){// constructor
  if (!a[3]) {x=y=0; return;}
  const double fa[3]={double(a[0])/a[3],double(a[1])/a[3],double(a[2])/a[3]};
  x=redondea(fa[0]*p0[0]+fa[1]*p1[0]+fa[2]*p2[0]);
  y=redondea(fa[0]*p0[1]+fa[1]*p1[1]+fa[2]*p2[1]);
}
  // calcula las tres coordenadas de area
void p2e::fforma(const p2e &p0, const p2e &p1, const p2e &p2, long long *a, bool area_calculada) const {
  a[0]=((p1-*this)%(p2-*this))<<1;
  a[1]=((p2-*this)%(p0-*this))<<1;
  a[2]=((p0-*this)%(p1-*this))<<1;
  if (!area_calculada) a[3]=a[0]+a[1]+a[2];
}
