#include <fstream>
#include "delaunay.h"

using namespace std;

bool graba(const char *fname, const delaunay &D, const pila_ptr<p2e> &puntos){
  int i;
  ofstream f(fname);
  if (!f.is_open()) return false;
  for (i=0;i<D.deep;i++){
    const Dtri &t=D[i];
    f << &t << "\t"
      << t.p[0] << ","
      << t.p[1] << ","
      << t.p[2] << "\t"
      << t.vecino[0] << ","
      << t.vecino[1] << ","
      << t.vecino[2] << endl;
  }
  f.close();
  return true;
}


bool lee(const char *fname){
  ifstream f(fname);
  if (!f.is_open()) return false;
//  for i=0;i<
  return false;
}