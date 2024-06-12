#ifndef MATRIX_H
#define MATRIX_H
#include <cmath>
#include <cstring>
#include "globals.hpp"

// matrix de transformacion (3x3)
matrix::matrix(){std::memset(m,0,sizeof(m));}

void matrix::apply (GLfloat p[][4], GLfloat r[][4], int n) const { // aplicar transformaciones a los vectores
	for (int i=0;i<n;i++) {
		for (int j=0;j<3;j++) {
			r[i][j]=0;
			for (int k=0;k<3;k++)
				r[i][j]+=p[i][k]*m[j][k];
		}
		r[i][3]=r[i][2];
	}
}

matrix matrix::operator*(const matrix &m2) const { // postmultiplicar por otra matrix
	matrix res;
	for (int i=0;i<3;i++)
		for (int j=0;j<3;j++)
			for (int k=0;k<3;k++)
				res[i][j]+=m[i][k]*m2[k][j];
	return res;
}

// muestra los valores de la matrix
// calcula las matrices y las aplica a los puntos del triangulo
void update_points_and_matrix() {
	double cosp=cos(tp);
	double sinp=sin(tp);
	double cosr=cos(tr);
	double sinr=sin(tr);
	
	// shear y escalado
	matrix mse;
	mse[0][0]=ts*sx;
	mse[1][1]=ts*sy;
	mse[2][2]=ts*tw;
	mse[0][2]=tx;
	mse[1][2]=ty;
	
	// rotacion sobre el eje x
	matrix mrx;
	mrx[1][1]=cosp;
	mrx[1][2]=-sinp;
	mrx[0][0]=1;
	mrx[2][1]=sinp;
	mrx[2][2]=cosp;
	
	// rotacion sobre el eje w
	matrix mrw;
	mrw[0][0]=cosr;
	mrw[0][1]=-sinr;
	mrw[1][0]=sinr;
	mrw[1][1]=cosr;
	mrw[2][2]=1;
	
	mat=mrw*(mse*mrx);
	
	mat.apply(quad?pts_quad:pts_tri,pts_trans,quad?4:3);
	memcpy(pts_trans+4,pts_trans,sizeof(*pts_trans)*4);
	for(int i=4;i<8;i++) for(int j=0;j<4;j++) pts_trans[i][j]=-pts_trans[i][j];
	
	static GLfloat bbt[8][4]={
		{-1,-1,0,0},
		{-1,+1,0,0},
		{+1,+1,0,0},
		{+1,-1,0,0},
		{-1,-1,2,0},
		{-1,+1,2,0},
		{+1,+1,2,0},
		{+1,-1,2,0}
	};
	static GLfloat bbq[8][4]={
		{-.8,-1.2,0,0},
		{-.8,+1.2,0,0},
		{+.8,+1.2,0,0},
		{+.8,-1.2,0,0},
		{-.8,-1.2,2,0},
		{-.8,+1.2,2,0},
		{+.8,+1.2,2,0},
		{+.8,-1.2,2,0}
	};
	mat.apply(quad?bbq:bbt,pts_bb,8);
	
}


#endif
