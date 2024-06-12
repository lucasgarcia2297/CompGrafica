#ifndef NURBS_H
#define NURBS_H

#include <cstring>
#include <fstream>

#define _expects(cond) { if (!(cond)) asm("int3"); asm("nop"); }

#define ARRAY_MAX 1000
#define ZOOM_FACTOR 1.1
#define SEL_NONE -1
#define SEL_NEW -2

enum { X=0, Y=1, Z=2, W=3 };
enum {
	KD_UNIFORM,
	KD_BEZIER_BOUNDARY,
	KD_BEZIER_SPLINE
};


struct NURBS {
	GLfloat controls[ARRAY_MAX][4]; // puntos de control (x,y,z,w)
	GLfloat knots[ARRAY_MAX]; // knots
	int num; // cantidad de pts de control
	int order; // orden de la curva ( = grado+1 )
	int knum; // cantidad de knots ( = num + order )
	float detail; // tolerancia para el rasterizado de opengl
	int knots_points[ARRAY_MAX][2]; // coord donde se dibujan los knots en la ventana
	GLfloat aux_c[10][10][4]; // auxiliar (para InsertKnot)
	
	NURBS &operator=(const NURBS &n2) = delete;
	NURBS(const NURBS &n2) {
		detail=n2.detail;
		num=n2.num;
		knum=n2.knum;
		order=n2.order;
		std::memcpy((void*)controls,(void*)n2.controls,sizeof(float)*4*num);
		std::memcpy((void*)knots,(void*)n2.knots,sizeof(float)*ARRAY_MAX);
	}
	
	NURBS () {
		detail=25;
		Clear();
	}
	
	void Move(int x, int y) {
		for (int i=0;i<num;i++) {
			float w=controls[i][W];
			controls[i][X]+=w*x;
			controls[i][Y]+=w*y;
		}
	}
	
	void Clear() {
		num=0;
		knum=order=4;
		knots[0]=0; knots[order-1]=1;
		for (int i=0;i<order;i++)
			knots[i]=(i-1)*1.f/(order-3);
		for (int i=order-1;i<ARRAY_MAX;i++)
			knots[i]=1;
	}
	
	
	
	// para invertir el sentido de la curva se el orden los ptos de control
	// y se modifica el vector de knots de forma que las distancias entre knots
	// consecutivos queden invertidas (ej { .1, .3, 1 }  -> { 0, .7, .9 }
	
	void InvertCurve() {
		GLfloat t;
		for (int j=0;j<4;j++) {
			for (int i=0;i<num/2;i++) {
				t=controls[i][j];
				controls[i][j]=controls[num-1-i][j];
				controls[num-i-1][j]=t;
			}
		}
		for (int i=0;i<knum/2;i++) {
			t=knots[i];
			knots[i]=1-knots[knum-1-i];
			knots[knum-i-1]=1-t;
		}
	}
	
	
	
	// para saber que punto esta cerca del cursor se hace va dividiendo en dos la curva
	// (para esto se insertan tantos knots como sea necesario hasta que interpole)
	// y se analiza en cual de las dos partes (o ambas) podria estar, utilizando el echo
	// de que todo punto de la curva esta dentro del convex hull de sus puntos de control,
	// y asi recursivamente se va acotando el espacio de busqueda hasta que mida 1 pixel
	
	float FindU(int x, int y, float tol=25) {
		_expects(order>0&&order<1e3);
		_expects(num>=0&&num<1e3);
		if (num<order) return SEL_NONE;
		NURBS n2(*this);
		_expects(n2.order==order&&n2.num==num);
		int k1 = n2.InsertKnot(knots[order-1],true);
		_expects(n2.order==order&&n2.num<num+order);
		int k2 = n2.InsertKnot(knots[knum-order],true);
		_expects(n2.order==order&&n2.num<num+2*order);
		float u = n2.FindUbb(n2,x,y,knots[order-1],knots[knum-order],k1-order+1,k2-order+1,tol);
		return u;
	}
	
	
	// esta func verifica si el pto que se busca esta dentro o cerca del bb
	// (voy a usar el bb en lugar del ch para simplificar la implementacion)
	// si el bb es suficientemente chico encontramos u, sino se divide la 
	// curva en dos y se lanza recursivamente
	
	static float FindUbb(NURBS &n2, const int &x, const int &y, float from, float to, int cfrom, int cto, float &tol) {
		
		GLfloat mx,my,Mx,My;
		mx=Mx=n2.controls[cfrom][X]/n2.controls[cfrom][W];
		my=My=n2.controls[cfrom][Y]/n2.controls[cfrom][W];
		
		GLfloat tx,ty;
		for (int i=cfrom;i<=cto;i++) {
			tx=n2.controls[i][X]/n2.controls[i][W];
			ty=n2.controls[i][Y]/n2.controls[i][W];
			if (ty<my) my=ty;
			if (ty>My) My=ty;
			if (tx<mx) mx=tx;
			if (tx>Mx) Mx=tx;
		}
		if (Mx-mx<=1 && My-my<=1) { // si el bb esta al limite (acotamos el pixel)
			float dx = (mx+Mx)/2-x;
			float dy = (my+My)/2-y;
			dy*=dy; dx*=dx;
			if (dx+dy<=tol) {
				tol = dx+dy;
				return (from+to)/2;
			} else {
				return SEL_NONE;
			}
		} else if (mx-tol<=x && Mx+tol>=x && my-tol<=y && My+tol>=y) { // divide & conquer
			float t = (from+to)/2; int num_insertados;
			int nc = n2.InsertKnot(t,true,&num_insertados);
			if (!num_insertados) return SEL_NONE; // sin este if, si hay discontinuidad, el bb no se achica más y revienta por recursión infinita
			float u1 = n2.FindUbb ( n2, x, y, t, to, nc-n2.order+1, cto+num_insertados, tol );
			float u2 = n2.FindUbb ( n2, x, y, from, t, cfrom, nc-n2.order+1, tol );
			return u2!=SEL_NONE?u2:u1;
		}
		return SEL_NONE;
		
	}
	
	
	// se realiza virtualmente el algoritmo de insercion de knots tantas veces hasta que
	// la curva interpole al punto de control (que inserto dicho algoritmo), obteniendo
	// a partir de este las coordenadas del punto de la curva
	
	GLfloat *FindPoint(float t, int *rep=nullptr) { // De Boor's Algorithm
		
		int degree = order -1;
		
		// buscar donde va el knot
		int k=0; while (k<knum && knots[k]<t) k++;
		// cuantas veces se repite
		int s=0; while (k<knum && knots[k]==t) { k++; s++; } k--;
		// corregir el extremo inferior para que no se salga
		if(degree-s<1) s=degree-1;
		// inicializar el vector auxiliar donde se van a guardar los pasos
		// (solo se guardan los ultimos dos, alternando entre las cols 0 y 1)
		for (int i=k-degree,p=0;i<=k-s+2;i++,p++) {
			int j = i>=num ? num-1 : i;
			aux_c[0][p][X]=controls[j][X];
			aux_c[0][p][Y]=controls[j][Y];
			aux_c[0][p][Z]=controls[j][Z];
			aux_c[0][p][W]=controls[j][W];
		}
		// iterar hasta llegar al punto
		for (int rr=1;rr<=degree-s;rr++) {
			for (int i=k-degree+rr,p=rr;i<=k-s;i++,p++) {
				// corregir el extremo superior para que no se salga
				float a = (knots[i+degree-rr+1]-knots[i])==0 ? 1 : (t-knots[i])/(knots[i+degree-rr+1]-knots[i]);
				aux_c[rr][p][X]=(1-a)*aux_c[rr-1][p-1][X]+a*aux_c[rr-1][p][X];
				aux_c[rr][p][Y]=(1-a)*aux_c[rr-1][p-1][Y]+a*aux_c[rr-1][p][Y];
				aux_c[rr][p][Z]=(1-a)*aux_c[rr-1][p-1][Z]+a*aux_c[rr-1][p][Z];
				aux_c[rr][p][W]=(1-a)*aux_c[rr-1][p-1][W]+a*aux_c[rr-1][p][W];
			}
		}
		if (rep) *rep=s;
		return aux_c[degree-s][degree-s];
	}
	
	
	// para agregar un pto al final, se agrega el pto de control a la lista y 
	// se escalan los ktnos (para mantener la forma de la curva)
	
	int AddControlPoint(GLfloat x, GLfloat y) {
		// order = degree+1 = knum-cnum => knum = order+cnum
		controls[num][X]=x;
		controls[num][Y]=y;
		controls[num][Z]=0;
		controls[num][W]=1;
		for (int i=0;i<knum-1;i++) {
			knots[i]*=1-1.f/(knum-2);
		}
		num++; knum++;
		return num-1;
	}
	
	
	// algoritmo de De Boor 
	// interpolate = true inserta tantas veces como sea necesario hasta que haya 
	// un pto de control en esa posicion de la curva (continuidad c0)
	// interpolate = false agrega una sola vez siempre y cuando la cantidad de veces
	// que ya esta presente no sea mayor al orden (se puede asi llegar a perder
	// la continuidad c0, pero luego ya no tiene sentido seguir insertando)
	// esto modifica el poligono de control, no la curva
	
	int InsertKnot(float t, bool to_interpolate = false, int *num_insertados = nullptr) {
		
		if (num_insertados) *num_insertados = 0;
		int k=0; // buscar donde va el knot
		int s=0; // y cuantas veces ya esta repetido si esta
		while (k<knum && knots[k]<t)
			k++;
		while (k<knum && knots[k]==t) {
			k++;
			s++;
		}
		if (k!=0) k--;
		
		if (s>=order-(to_interpolate?1:0)) {
			return k;
		}
		
		do {
			if (num_insertados) ++(*num_insertados);
			// hacer lugar para el nuevo knot
			for (int i=knum;i>k;i--) 
				knots[i+1]=knots[i];
			// insertar el knot
			knots[k+1]=t;
			
			// hacer lugar para el nuevo punto de control
			for (int i=knum;i>k;i--) {
				controls[i][X]=controls[i-1][X];
				controls[i][Y]=controls[i-1][Y];
				controls[i][Z]=controls[i-1][Z];
				controls[i][W]=controls[i-1][W];
			}
			// acomodar los pts de control que corresponda
			GLfloat lc[4]={0};
			lc[X]=controls[k-order+1][X];
			lc[Y]=controls[k-order+1][Y];
			lc[Z]=controls[k-order+1][Z];
			lc[W]=controls[k-order+1][W];
			for (int i=k-order+1;i<=k;i++) {
				for (int j=0;j<4;j++) {
					float a = (t-knots[i]) / (knots[i+order]-knots[i]) ;
					GLfloat tmp=controls[i][j];
					controls[i][j]=(1-a)*lc[j]+a*tmp;
					lc[j]=tmp;
				}
			}
			
			knum++;
			num++;
			k++;
			s++;
		} while (to_interpolate && s+1<order);
		return k;
	}
	
	
	// mueve knots de forma tal que la curva se "deforme" para interpolar el
	// pto de control
	
	void Interpolate(int c) {
		
		int i=c+1;
		if (i<0)
			i=0;
		float l1=0,l2=1;
		if (i>0)
			l1=knots[i-1];
		if (i+order<knum)
			l2=knots[i+order];
		float kn=(l2+l1)/2;
		for (int j=0;j<order-1;j++) {
			knots[i+j]=kn;
		}
	}
	
	
	// cambia el grado de la curva (sin consideraciones, la curva cambia)
	
	void SetDegree(int n) {
		order=n+1;
		knum=order+num;
		ResetKnots(true);
	}
	
	
	// elimina un pto de control y su knot "correspondiente"
	
	void DeleteControl(int n) {
		num--;
		knum--;
		for (int i=n;i<num;i++) {
			controls[i][X]=controls[i+1][X];
			controls[i][Y]=controls[i+1][Y];
			controls[i][Z]=controls[i+1][Z];
			controls[i][W]=controls[i+1][W];
		}
		for (int i=n;i<knum;i++)
			knots[i]=knots[i+1];
		knots[knum]=1;
	}
	
	
	// reacomoda los knots equiespaciadamente 
	// o interpolando los extremos y en medio equiespaciadamente
	
	void ResetKnots(int distribution=KD_UNIFORM) {
		if (distribution==KD_BEZIER_BOUNDARY) {
			int c=0;
			while (c<order)
				knots[c++]=0;
			while (c<knum-order) {
				knots[c]=float(c-order+1)/(knum-order-order+1);
				c++;
			}
			while (c<knum)
				knots[c++]=1;
		} else if (distribution==KD_BEZIER_SPLINE) {
			int rknum = knum-2, degree = order-1;
			int ntramos=rknum/degree+((rknum%degree)?1:0)-1, itramo=0, c=0;
			knots[c++]=0;
			while (c<=rknum) {
				if ((c-1)%degree==0) ++itramo;
				knots[c++] = float(itramo-1)/ntramos;
			}
			knots[c++]=1;
		} else /*if (distribution==KD_UNIFORM)*/ {
			for (int i=1;i<knum-1;i++)
				knots[i]=(i-1)*(1.f/(knum-3));
		}
	}
	
	// simulan el zoom aplicando un factor de escala a las coord de los pts de controls
	// y desplazandolos para que el mouse quede en el mismo lugar con respecto a la curva
	void ZoomIn(int x, int y) {
		for (int i=0;i<num;i++) {
			float w=controls[i][W];
			controls[i][X] = (controls[i][X]/w*ZOOM_FACTOR + (x-x*ZOOM_FACTOR))*w;
			controls[i][Y] = (controls[i][Y]/w*ZOOM_FACTOR + (y-y*ZOOM_FACTOR))*w;
		}
	}
	void ZoomOut(int x, int y) {
		for (int i=0;i<num;i++) {
			float w=controls[i][W];
			controls[i][X] = (controls[i][X]/w/ZOOM_FACTOR + (x-x/ZOOM_FACTOR))*w;
			controls[i][Y] = (controls[i][Y]/w/ZOOM_FACTOR + (y-y/ZOOM_FACTOR))*w;
		}
	}
	
	
	// dibuja la curva, el poligono de control y los pts
	
	bool WellDefined() { 
		return !(num<order || knots[order-1]==knots[num]);
	}
	
	// guardar/cargar los datos de la nurb en un archivo de texto 
	// (es el que se le pasa como parametro al ejecutable)
	void Save(const std::string &fname) {
		std::ofstream fil(fname.c_str(),std::ios::trunc);
		fil << "order " << order << std::endl;
		fil << "pts " << num << std::endl;
		int i;
		for (i=0;i<num;i++)
			fil << controls[i][X] << " " << controls[i][Y] << " " 
				<< controls[i][Z] << " " << controls[i][W] << std::endl;
			fil << "knots " << knum << std::endl;
			for (i=0;i<knum;i++)
				fil << knots[i] << std::endl;
			fil << "detail " << detail << std::endl;
			fil.close();
	}
	bool Load(const std::string &fname) {
		std::ifstream fil(fname.c_str());
		if (fil.is_open()) {
			std::string s;
			fil >> s >> order;
			fil >> s >> num;
			int i;
			for (i=0;i<num;i++)
				fil >> controls[i][X] >> controls[i][Y] 
					>> controls[i][Z] >> controls[i][W];
				fil >> s >> knum;
				for (i=0;i<knum;i++)
					fil >> knots[i];
				fil >> s >> detail;
				return true;
		}
		return false;
	}
	
	void CalcKNots() {
		// (por alguna razon los de los extremos (p primeros y p ultimos) no tienen sentido, asi que no se dibujan
		for (int i=order;i<knum-order;i++) {
			float *p=FindPoint(knots[i]);
			knots_points[i][X]=int(p[X]/p[W]);
			knots_points[i][Y]=int(p[Y]/p[W]);
		}
	}
	
};

#endif
