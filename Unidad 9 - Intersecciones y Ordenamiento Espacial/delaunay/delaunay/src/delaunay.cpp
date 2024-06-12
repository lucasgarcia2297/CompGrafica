////////////////////////////////////////////////////////////////////////
//////////////////   TRIANGULACION DELAUNAY 2D   ///////////////////////
////////////////////////////////////////////////////////////////////////

#include "delaunay.h"

// Calcula si un punto esta o no dentro del circulo
      //   C=c-p0=cc/aa; PT=pt-p0;
      //   (PT-C)^2 <? r^2
      //   r^2 = C^2 (origen en uno de los puntos)
      //   (PT-C)^2 <? C^2
      //   PT^2 - 2 PT.C - C^2 <? C^2
      //   PT^2 <? 2 PT.C

// en el circulo o el borde (<=)
bool Dtri::tiene_c(const p2e &pt) const{
  p2e PT=pt-*(p[0]);
  return (PT.mod2()*aa<=(PT[0]*cc[0]+PT[1]*cc[1])*2);
}
// interior estricto (<)
bool Dtri::contiene_c(const p2e &pt) const{
  p2e PT=pt-*(p[0]);
  return (PT.mod2()*aa<(PT[0]*cc[0]+PT[1]*cc[1])*2);
}

const int delaunay::tol=5;

delaunay::~delaunay(){// destructor
  while ((--deep)>=0) delete list[deep]; // borra los triangulos
  // de destruir list se encarga el destructor de pila
}

// inicializa con cuatro puntos del bbox en circulo (00 10 11 01)
void delaunay::init(const p2e &p00,const p2e &p10,const p2e &p11,const p2e &p01){
  while ((--deep)>=0) delete list[deep]; deep=0; // borra lo que hubiera de antes
  ll=p00; ur=p11;
  Dtri *t0=new Dtri(p00,p10,p01); push(t0);
  Dtri *t1=new Dtri(p11,p01,p10); push(t1);
  t0->vecino[0]=t1;t0->vecino[1]=0;t0->vecino[2]=0;
  t1->vecino[0]=t0;t1->vecino[1]=0;t1->vecino[2]=0;
}

// verifica si esta dentro del bounding box
bool delaunay::test_en_box(const p2e &p) const{
  if (p[0]-ll[0]<tol||p[1]-ll[1]<tol||
      ur[0]-p[0]<tol||ur[1]-p[1]<tol) return false;
  return true;
}

/* Swap de diagonales

                   ix
                  / \                              / \
     (ix+2)%3   /     \    (ix+1)%3              / 2|1 \
              /    tv   \                      /    |    \
            /             \                  /      |      \
   (i+2)%3 <---------------> (i+1)%3        < 0 ntv | nt   0>
            \             /                  \      |      /
              \    t    /                      \    |    /
      (i+1)%3   \     /    (i+2)%3               \ 1|2 /
                  \ /                              \ /
                   i

*/
static void Dswap(Dtri &t, Dtri &tv){  
  int i=t.indice_vecino(tv),ix=tv.indice_vecino(t);
  // construye los nuevos triangulos
  Dtri nt(t[(i+1)%3],tv[ix],t[i]); // este reemplazara a t
  Dtri ntv(t[(i+2)%3],t[i],tv[ix]); // este reemplazara a tv
  // arrega vecindades (punteros a triangulos)
  nt.vecino[0]=&tv; ntv.vecino[0]=&t;
  // estos dos siguen siendo vecinos de t y tv respectivamente
  nt.vecino[1]=t.vecino[(i+2)%3]; ntv.vecino[1]=tv.vecino[(ix+2)%3];
  // los otros dos cambian
  Dtri *v;
  v=tv.vecino[(ix+1)%3]; nt.vecino[2]=v; if (v) v->reemplaza_vecino(tv,t);
  v=t.vecino[(i+1)%3]; ntv.vecino[2]=v; if (v) v->reemplaza_vecino(t,tv);
  // reemplaza
  t=nt; tv=ntv;
}

// Restaura la triangulacion para que siga siendo Delaunay cuando algo cambio
// En revisar estan los triangulos que se sabe que hay que revisar y se agregan los que se modifican
// Labura mientras haya triangulos para revisar
// El flag es para no poner un triangulo que ya esta en la pila
static void Drestaura(pila_ptr<Dtri> &revisar){
  static const int en_revisar=1;
  int i;
  // asigna el flag a los que ya estan dentro
  for (i=0;i<revisar.deep;i++) revisar[i].f|=en_revisar;
  // revisa
  while (revisar.deep) {
    Dtri &t=revisar[--revisar.deep]; // referencia y lo saca de la pila
    t.f&=~en_revisar; // saca el flag de que esta en la lista
    // verifica contra los tres vecinos si cumplen Delaunay
    i=2; do{
      Dtri *v=t.vecino[i];
      if (!v) continue; // frontera
      Dtri &tv=*v;
      // verifica si cumplen delaunay
      if (!tv.contiene_c(t[i])) continue; // cumplen
      // no cumplen
      revisar.deep++; t.f|=en_revisar; // repone t en la pila
      if (!(tv.f&en_revisar)) {revisar.push(tv); tv.f|=en_revisar;} // agrega tv (si no estaba)
      Dswap(t,tv); // swappea diagonales
      break; // t ya cambio => no sigue el loop en i
    }while (i--);
  }
}

  // busca los elementos que tienen el punto
bool delaunay::cluster(const p2e &p, pila_ptr<Dtri> &ptris){
  int ix=3,i=deep;

  // busca solo el primero aqui
  while (i&&(ix==3)) ix=list[--i]->indice(p);
  if (ix==3) return false; // ningun elemento tiene al punto (aislado??)

  // el i-esimo tiene al punto
  Dtri *t=list[i],*t0=t;

  ptris.push(t0); // triangulos con p
  // aqui busca los siguientes por vecindades
  t=t0; while((t=t->vecino[(ix+1)%3])!=t0){ix=t->indice(p); ptris.push(t);}
  return true;
}

// Busca en que triangulo esta el punto
// Se asume que esta en alguno!!
// Empieza desde start o uno cualquiera
// a son las funciones de forma del punto en el triangulo
Dtri& delaunay::enquetriangulo(const p2e &p, long long a[4], Dtri *start){
  static const int visitado=1; //////////////////////////////////////////////////////////////////////////////////////////////sacar
  pila_ptr<Dtri> visitados; // guarda en una pila para limpiar el flag antes de salir
  Dtri *t=start; if (!t) t=list[deep-1]; // desde start o desde el ultimo
  t->fforma(p,a);
  // busca la funcion de forma mas negativa
  long long amin=a[0], imin=0;
  if (a[1]<amin) {amin=a[1],imin=1;}
  if (a[2]<amin) {amin=a[2],imin=2;}
  while (amin<0) { // significa que no esta dentro del triangulo)
    t->f|=visitado; visitados.push(t);
    t=t->vecino[imin];// -> pasa al vecino opuesto al punto mas alejado
    // y recalcula
    t->fforma(p,a);
    amin=a[0]; imin=0;
    if (a[1]<amin) {amin=a[1],imin=1;}
    if (a[2]<amin) {amin=a[2],imin=2;}
  }
  // limpia flags
  int &vd=visitados.deep; while(--vd>=0){visitados[vd].f&=~visitado;}
  return *t;
}

// agrega un punto y repone Delaunay
// si esta muy cerca de otro no agrega
// si esta en la frontera de un triangulo lo mueve hacia dentro
bool delaunay::agregapunto(p2e &p){
  if (!test_en_box(p)) return false;
  // seguro que esta dentro de un triangulo
  long long a[4], amin, imin, amed, imed, amax, imax;
  Dtri &t=enquetriangulo(p,a); // el punto esta en t 

  // puede estar en la frontera o muy cerca de un nodo
  // ordena las funciones de forma
  amin=a[0], imin=0, amed=a[1], imed=1, amax=a[2], imax=2;
  if (amed>amax){intercambia(amed,amax); intercambia(imed,imax);}
  // med<=max
  if (amin>amed){
    intercambia(amin,amed); intercambia(imin,imed);
    // amin es la menor
    if (amed>amax){intercambia(amed,amax); intercambia(imed,imax);}
  }

  // verifica distancia al mas cercano
  if (p.distanciac(t[imax])<tol) return false;

  // si esta en la frontera lo mueve una unidad dentro 
  // para evitar problemas (triangulo de area nula)
  while (!amin){
    // lo mueve hacia el que tiene funcion de forma nula
    p2e dir=t[imin]-p;
    // una unidad en direccion dir
    if (abs(dir[0])>abs(dir[1]<<1)) // domina x
      {dir[1]=0; dir[0]=(dir[0]>0)? 1: -1;}
    else if (abs(dir[1])>abs(dir[0]<<1)) // domina y
      {dir[0]=0; dir[1]=(dir[1]>0)? 1: -1;}
    else // parejo
      {dir[0]=(dir[0]>0)? 1: -1; dir[1]=(dir[1]>0)? 1: -1;}
    p+=dir; // perturba el punto
    p.fforma(t[0],t[1],t[2],a,true);
    amin=minimo(a[0],minimo(a[1],a[2]));
  }

  // esta dentro
  // une el punto con los tres vertices y forma dos nuevos triangulos
  Dtri *nuevo[2]; // copia local de t porque lo va a modificar
  nuevo[0]=new Dtri(p,t[2],t[0]); push(nuevo[0]);
  nuevo[1]=new Dtri(p,t[0],t[1]); push(nuevo[1]);
  t.p[0]=&p; c3(t.p,t.cc,t.aa); // recalcula el viejo porque cambio un punto

  // arregla los vecinos
  Dtri *v;
  nuevo[0]->vecino[0]=v=t.vecino[1]; if (v) v->reemplaza_vecino(t,*nuevo[0]);
  nuevo[0]->vecino[1]=nuevo[1]; 
  nuevo[0]->vecino[2]=&t;

  nuevo[1]->vecino[0]=v=t.vecino[2]; if (v) v->reemplaza_vecino(t,*nuevo[1]);
  nuevo[1]->vecino[1]=&t; 
  nuevo[1]->vecino[2]=nuevo[0];

  t.vecino[1]=nuevo[0];
  t.vecino[2]=nuevo[1];

  // Restaura Delaunay
  pila_ptr<Dtri> revisar;
  revisar.push(nuevo[0]); revisar.push(nuevo[1]); revisar.push(t);
  Drestaura(revisar);
  return true;
}

// retestea delaunay al mover un punto
bool delaunay::muevepunto(p2e &p, const p2e &newpos){
  int i,j;
  if (newpos==p) return true;
  if (!test_en_box(newpos)) return false;
  
  // busca los elementos que tienen el punto
  pila_ptr<Dtri> ptris;
  cluster(p,ptris);
  // verifica que no este demasiado cerca de otro
  for(i=0;i<ptris.deep;i++){
    const Dtri &t=ptris[i];
    if (&t[0]!=&p && newpos.distanciac(t[0])<tol) return false;
    if (&t[1]!=&p && newpos.distanciac(t[1])<tol) return false;
    if (&t[2]!=&p && newpos.distanciac(t[2])<tol) return false;
  }

  // actualiza posicion (pero guarda la vieja por las dudas)
  p2e oldp(p); p=newpos; 
  // recalcula las circunferencias 
  // y verifica si hay area negativa (movimiento brusco)
  for(i=0;i<ptris.deep;i++){
    Dtri &t=ptris[i];
    c3(t.p,t.cc,t.aa);
    if (t.aa<=0) { // area negativa o nula
      // restituye todo como estaba y se va
      p=oldp; // vuelve el punto a su pos original
      // restaura circulos modificados
      for (j=0;j<=i;j++){
        Dtri &o=ptris[j];
        c3(o.p,o.cc,o.aa);
      }
      return false;
    }
  }

  // todo bien asi que restaura Delaunay
  Drestaura(ptris);
  return true;
}

  // elimina un punto de la triangulacion
bool delaunay::quitapunto(const p2e &p){
  // busca los elementos que tienen el punto
  pila_ptr<Dtri> ptris; cluster(p,ptris);
  pila_ptr<Dtri> revisar(ptris.deep);
  // elimina aristas swapeando pares cuando las diagonales se cruzan
  int j,i,ix;
  while (ptris.deep>3){
    for (j=0;j<ptris.deep;j++){
      // en el swap es como que los triangulos "giran a la izquierda";inn
      // necesito que el tv y no el t sea el que quede con p,
      // ==> busco el vecino adecuado (indice de p + 1)
      Dtri &t=ptris[j]; i=t.indice(p); // i=indice del punto en t      
      Dtri &tv=t.v((i+1)%3); ix=tv.indice(p); // ix=indice del punto en tv
      if (!hay_interseccion_estricta(p,t[(i+2)%3],t[(i+1)%3],tv[(ix+2)%3])) continue;
      Dswap(t,tv); // tv queda con p y queda en ptris
      revisar.push(t); // t se swappeo => hay que revisarlo (despues)      
      ptris.remove(j); // t desaparece de ptris
    }
  }
  // quedan 3, reemplaza, recalcula y elimina dos triangulos
  Dtri *v;
  Dtri &t=ptris[0]; i=t.indice(p); // este queda
  // vuela el vecino i+1
  Dtri &t1=t.v((i+1)%3); ix=t1.indice(p);
  t.p[i]=t1.p[(ix+2)%3]; c3(t.p,t.cc,t.aa); // reemplaza y recalcula
  v=t.vecino[(i+1)%3]=t1.vecino[ix]; if (v) v->reemplaza_vecino(t1,t);
  remove(t1); delete &t1;
  // vuela el vecino i+2
  Dtri &t2=t.v((i+2)%3); ix=t2.indice(p);
  v=t.vecino[(i+2)%3]=t2.vecino[ix]; if (v) v->reemplaza_vecino(t2,t);
  remove(t2);  delete &t2;
  
  revisar.push(t);
  Drestaura(revisar);
  return true;
}

