#ifndef MATRIX_HPP
#define MATRIX_HPP

// matriz de transformacion (3x3)
struct matrix { 
	GLfloat m[3][3];
	matrix();
	GLfloat *operator[](int i) { return m[i]; }
	const GLfloat *operator[](int i) const { return m[i]; }
	void apply (GLfloat p[][4], GLfloat r[][4], int n) const; // aplicar transformaciones a los vectores
	matrix operator*(const matrix &m2) const; // postmultiplicar por otra matriz
};

#endif
