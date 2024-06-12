#ifndef _UTILES_
#define _UTILES_

#include <cstdlib> // malloc
#include <cstddef> // size_t

static const int SZptr=sizeof(void *);
static const int SZint=sizeof(int);
static const size_t SZB=sizeof(bool);
static const size_t SZS=sizeof(short);
static const size_t SZI=sizeof(int);
static const size_t SZD=sizeof(double);
static const size_t SZP=sizeof(void *);

// cambia el contenido y no la direccion (lento pero seguro)
template <class T> static inline void intercambia(T&t1, T&t2) //swap en conflicto con stl
  {T t0=t1; t1=t2; t2=t0;}

// mínimo y maximo
template <class T> static inline const T &minimo(const T &t1,const T &t2)
  {if (t1<t2) return t1; else return t2;}
template <class T> static inline const T &maximo(const T &t1,const T &t2)
  {if (t1>t2) return t1; else return t2;}

// potencia >=0 (ojo, no testea <0)
template <class T> static inline T pown(const T &inp, int p)
  {T ret=1;  while (p--) ret*=inp; return ret;} // ojo el 1 en clases complicadas
template <class T> static inline T cuadrado(const T &inp)
  {return inp*inp;}

template <class T> static inline bool set_min(T& tmin,const T& tcompare)
  {if (tcompare<tmin) {tmin=tcompare; return true;} else return false;}
template <class T> static inline bool set_max(T& tmax,const T& tcompare)
  {if (tcompare>tmax) {tmax=tcompare; return true;} else return false;}
template <class T> static inline int set_min_max(T& tmin,T& tmax,const T& tcompare){
  if (tcompare>tmax) {tmax=tcompare; return 1;}
  if (tcompare<tmin) {tmin=tcompare; return -1;}
  return 0;
}

template <class T> static inline int redondea(const T& value)
{return (value>0)? (int(value+.5)): (int(value-.5));}
template <class T> static inline int redondea(const T& value,const T& r)
{return (value>0)? ((int(value/r+.5))*r): ((int(value/r-.5))*r);}

// guarda temporariamente el valor de un bool
#define _push_1b(nombre,valor) bool nombre##temp=nombre; nombre=valor;
#define _pop_1b(nombre) nombre=nombre##temp;

// raiz cuadrada entera
// s=(s+x/s)>>1, empezando de x/2 da el mayor entero cuyo cuadrado es < x
// yo calculo raiz de 4x, si es impar redondeo a mas y si es par a menos
static int raiz(long x){
  if (x<=0) return 0;
  long s=x<<1,sant=s<<1;
  x=sant;
  while (s<sant) {
    sant=s; 
    s=(s+x/s)>>1;
  }
  s=sant; s>>=1;
  if ((s<<1)!=sant) s++;
  return int(s); 
}

// pila de punteros
// al borrar no se borran los objetos sino la lista
template <class C> class pila_ptr{
protected:
  int size;
  C **list;

public:
  int deep; // cantidad de elementos efectivamente alojados
  // deep es publico: se puede manipular para restaurar la pila

  pila_ptr<C>(int sz=16){
    size=sz; deep=0;
    list=(C**)malloc(SZptr*size);
  }
  ~pila_ptr<C>(){free(list);}

  void push(C *p) { // agrega un puntero
    if (deep==size-1){
      size+=size>>1;
      if (!size) size=16;
      list=(C**)realloc(list,SZptr*size);
    }
    list[deep++]=p;
  }
  void push(C &o) {push(&o);} // agrega el puntero a o

  C* pop() {if (!deep) return 0; return list[--deep];} // saca el ultimo puntero

  C& operator[](int i){return *(list[i]);} // entrega el objeto i-esimo (0<=i<deep)
  const C& operator[](int i) const {return *(list[i]);} // entrega el objeto i-esimo (0<=i<deep)

  operator bool() {return (deep!=0);} // para saber si tiene algo

  bool remove(const C *p){ // elimina un puntero
    int i=deep; while(--i>=0) {
      if (list[i]!=p) continue;
      list[i]=list[--deep];
      return true;
    }
    return false;
  }
  bool remove(const C &o){return remove(&o);} // elimina el puntero a un objeto
  void remove(int i){list[i]=list[--deep];} // elimina el puntero al iesimo objeto
};

#endif
