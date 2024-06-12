////////////////////////////////////////////////////////////////////////
//////////////////   TRIANGULACION DELAUNAY 2D   ///////////////////////
////////////////////////////////////////////////////////////////////////

#ifndef _DELAUNAY_
#define _DELAUNAY_

#include "p2e.h"

// Triangulo Delaunay 
class Dtri{
public:
  const p2e *p[3];  // punteros (variables) a puntos (intocables desde aca)
  Dtri *vecino[3];  // punteros a vecinos adyacentes (tocables) 0=frontera
  long long aa;          // cuadruple del area => a=aa/4
  long long cc[2];       // cc=aa*(c-p0) => c=p0+cc/aa
  int f;            // flag multiproposito (bit a bit)
   
  // constructores
//  Dtri(){p[0]=p[1]=p[2]=0; vecino[0]=vecino[1]=vecino[2]=0; aa=0; f=0;}
  Dtri(const Dtri& t){*this=t;}
  Dtri(const p2e &p0, const p2e &p1, const p2e &p2){
    p[0]=&p0; p[1]=&p1; p[2]=&p2;
    c3(p0,p1,p2,cc,aa);
    vecino[0]=vecino[1]=vecino[2]=0;
    f=0;
  }

  // copia
//  Dtri &operator=(const Dtri& t){
//    p[0]=t.p[0]; p[1]=t.p[1]; p[2]=t.p[2];
//    vecino[0]=t.vecino[0]; vecino[1]=t.vecino[1]; vecino[2]=t.vecino[2]; 
//    aa=t.aa; cc[0]=t.cc[0]; cc[1]=t.cc[1]; f=t.f;
//    return *this;
//  }

  const p2e& operator[](int i) const {return *(p[i]);}
  Dtri &v(int i) {return *(vecino[i]);}

  // funciones de forma
  void fforma(const p2e &pt, long long *ai) const{
    ai[3]=aa; pt.fforma(*p[0],*p[1],*p[2],ai,true);
  }

  // contencion en circulo o triangulo
  // si esta en la frontera devuelve true
  bool tiene_c(const p2e &pt) const; // circulo  
  bool tiene_t(const p2e &pt, long long *ai) const{ // triangulo (calcula las coords de area)
    ai[3]=aa; pt.fforma(*p[0],*p[1],*p[2],ai,true);
    if (ai[0]<0||ai[1]<0||ai[2]<0) return false;
    return true;
  }
  // si esta en la frontera devuelve false
  bool contiene_c(const p2e &pt) const; // circulo  
  bool contiene_t(const p2e &pt, long long *ai) const{ // triangulo (calcula las coords de area)
    ai[3]=aa; pt.fforma(*p[0],*p[1],*p[2],ai,true);
    if (ai[0]<=0||ai[1]<=0||ai[2]<=0) return false;
    return true;
  }

  int indice(const p2e &pt) const{ // si no esta devuelve 3
    if (&pt==p[0]) return 0;
    if (&pt==p[1]) return 1;
    if (&pt==p[2]) return 2;
    return 3;
  }
  int indice_vecino(const Dtri &t) const{ // si no esta devuelve 3
    if (&t==vecino[0]) return 0;
    if (&t==vecino[1]) return 1;
    if (&t==vecino[2]) return 2;
    return 3;
  }

  void reemplaza_vecino(Dtri &viejo, Dtri &nuevo){
    vecino[indice_vecino(viejo)]=&nuevo;
  }

  // calcula los verdaderos centro y radio
  bool circulo(p2e &c, int &r){
    if (!aa) return false;
    double xcd=double(cc[0])/aa, ycd=double(cc[1])/aa;
    r=raiz(redondea(cuadrado(xcd)+cuadrado(ycd)));
    c[0]=redondea(xcd+(*p[0])[0]);
    c[1]=redondea(ycd+(*p[0])[1]);
    return true;
  }
};

//===============================================================================
// Delaunay "es" una pila de triangulos con mas datos y funciones especificas
class delaunay: public pila_ptr<Dtri>{
public:
  p2e ll,ur; // bounding box (lower left, upper right)
  static const int tol; // minima distancia a otro punto o al bounding box
  
  delaunay(){};// constructor defalut sin nada

  // destructor (borra los triangulos)
  ~delaunay();

  // construye con cuatro puntos del bbox en orden circular (00 10 11 01)
  delaunay(const p2e &p00,const p2e &p10,const p2e &p11,const p2e &p01)
    {init(p00,p10,p11,p01);}

  // inicializa con cuatro puntos del bbox en orden circular (00 10 11 01)
  void init(const p2e &p00,const p2e &p10,const p2e &p11,const p2e &p01);

  // verifica si esta dentro del bounding box
  bool test_en_box(const p2e &p) const;

  // en que triangulo esta el punto (a=areas parciales y total para funciones de forma)
  Dtri& enquetriangulo(const p2e &p, long long a[4], Dtri *start=0);

  // busca los elementos que tienen el punto
  bool cluster(const p2e &p, pila_ptr<Dtri> &ptris);

  // agrega un punto y repone Delaunay
  // p no es const porque puede resultar perturbado
  bool agregapunto(p2e &p);

  // retestea delaunay al mover un punto
  bool muevepunto(p2e &p, const p2e &newpos);

  // elimina un punto de la triangulacion
  bool quitapunto(const p2e &p);
  
  // retorna true si es una de los puntos "virtuales"
  bool esvirtual(const p2e &p) const {
	  return p.x == ll.x||p.x==ur.x || p.y==ll.y||p.y==ur.y;
  }
};

#endif
