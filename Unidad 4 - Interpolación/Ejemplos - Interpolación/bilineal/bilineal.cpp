/**
* En este demo se muestra un cuadrilátero con colores en los vértices, y en puntos
* interpolados en el interior. Se pueden editar los vértices con el drag, cambiar
* la densidad de la "grilla" que se usa para el interior con + y -, alternar
* entre ver la grilla, o el cuadrilátero relleno con F, y cambiar el modo de
* interpolación para el interior con M. Un modo usa la bilineal en el quad,
* otro la hiperbólica ("inventando" unos zs/ws para los vertices) y los 
* otros lo parten en dos triángulos (la diferencia es cual diagonal usan)
* e interpolan linealmente dentro de los mismos. Con la "D" se remarcan las
* diagonales en el espacio u,v (esto es las curvas u=v, y u=1-v)
**/

#include <GL/glut.h>
#include "../common/OSD.hpp"
#include "../common/Algebra.hpp"

int sel=-1, win_w=640, win_h=480, n=15;
Punto pt[4] = { Punto{100,100}, Punto{100,380}, Punto{540,380}, Punto{540,100}};
using Color = Punto;
Color cl[4] = { Color{1,0.4,0}, Color{0.9,.75,0}, Color{0.5,0,0.5}, Color{0,0,1}};
bool filled=false, diag=false; 
enum { MDiagVA, MDiagRR, MBilineal, MHiperb };
int metodo = MBilineal;
static char smetodo[][64] = { "Diagonal Verde-Azul", "Diagonal Rojo-Rojo", "Bilineal", "Hiperbolilca" };
bool help = false;

void glColorPT(Punto p) { glColor3f(p.x,p.y,p.z); }
void glVertexPT(Punto p) { glVertex3f(p.x,p.y,p.z); }

Punto bilineal(const Punto pt[], float u, float v) {
	return interp2( interp2(pt[0],pt[1],v), interp2(pt[3],pt[2],v) ,u );
}

Punto lineal(const Punto pt[], float a0, float a1, int i0, int i1, int i2) {
	return interp3(pt[i0],pt[i1],pt[i2],a0,a1);
}

Punto hiperb(const float ws[], const Punto pt[], float a0, float a1, int i0, int i1, int i2) {
	float sum = ws[i0]*a0+ws[i1]*a1+ws[i2]*(1-a0-a1);
	return interp3(pt[i0],pt[i1],pt[i2],a0*ws[i0]/sum,a1*ws[i1]/sum);
}

void reshape_cb (int w, int h) {
	if (w==0||h==0) return;
	win_w = w; win_h = h;
	glViewport(0,0,w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,w,0,h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

Punto int_plano_recta(Punto a, Vector norm, Punto o, Punto c) {
	float ao = (o-a)*norm;
	float ac = (c-a)*norm;
	return interp2(o,c,ao/(ao-ac));
	
}

void calc_fake_z(const Punto pt[], float *w, Punto &idiags) {
	
	idiags = int_plano_recta(pt[0],(pt[2]-pt[0])%Vector(0,0,1),pt[1],pt[3]);
	
	float d[4];
	for(int i=0;i<4;i++) 
		d[i] = (pt[i]-idiags).Mod();
	
	for(int i=0;i<4;i++)
		w[i] = (d[i]+d[(i+2)%4]) / d[i];
	
}


void display_cb() {
	glClear(GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK,filled?GL_FILL:GL_LINE);
	
	float w[4]; Punto c; if (metodo==MHiperb)	calc_fake_z(pt,w,c);
	
	
	if (metodo==MDiagRR||metodo==MDiagVA) { // lineales
		
		
		glColor3f(0,0,0);
		glBegin(GL_TRIANGLES);
		for(int k=0;k<2;k++) { 
			int p0 = metodo==MDiagRR ? (k?0:2) : (k?1:3), 
				  p1 = metodo==MDiagRR ? (k?1:3) : (k?2:0), 
				  p2 = metodo==MDiagRR ? (k?2:0) : (k?3:1);
			for(int i=0;i<n;i++) { 
				float u0=1.f/(n)*i;
				float u1=1.f/(n)*(i+1);
				for(int j=0;j<n-i;j++) { 
					float v0=1.f/(n)*j;
					float v1=1.f/(n)*(j+1);
					glColorPT(lineal(cl,u0,v0,p0,p1,p2));
					glVertexPT(lineal(pt,u0,v0,p0,p1,p2));
					glColorPT(lineal(cl,u1,v0,p0,p1,p2));
					glVertexPT(lineal(pt,u1,v0,p0,p1,p2));
					glColorPT(lineal(cl,u0,v1,p0,p1,p2));
					glVertexPT(lineal(pt,u0,v1,p0,p1,p2));
					if (j==n-i-1) break;
					glColorPT(lineal(cl,u1,v1,p0,p1,p2));
					glVertexPT(lineal(pt,u1,v1,p0,p1,p2));
					glColorPT(lineal(cl,u1,v0,p0,p1,p2));
					glVertexPT(lineal(pt,u1,v0,p0,p1,p2));
					glColorPT(lineal(cl,u0,v1,p0,p1,p2));
					glVertexPT(lineal(pt,u0,v1,p0,p1,p2));
				}
				
			}
		}
		glEnd();
		
		
		
		
	} else if (metodo==MHiperb) {
		
		glColor3f(0,0,0);
		glBegin(GL_TRIANGLES);
		for(int k=0;k<2;k++) { 
			int p0=(k?1:3), p1=(k?2:0), p2=(k?3:1);
			for(int i=0;i<n;i++) { 
				float u0=1.f/(n)*i;
				float u1=1.f/(n)*(i+1);
				for(int j=0;j<n-i;j++) { 
					float v0=1.f/(n)*j;
					float v1=1.f/(n)*(j+1);
					glColorPT(hiperb(w,cl,u0,v0,p0,p1,p2));
					glVertexPT(hiperb(w,pt,u0,v0,p0,p1,p2));
					glColorPT(hiperb(w,cl,u1,v0,p0,p1,p2));
					glVertexPT(hiperb(w,pt,u1,v0,p0,p1,p2));
					glColorPT(hiperb(w,cl,u0,v1,p0,p1,p2));
					glVertexPT(hiperb(w,pt,u0,v1,p0,p1,p2));
					if (j==n-i-1) break;
					glColorPT(hiperb(w,cl,u1,v1,p0,p1,p2));
					glVertexPT(hiperb(w,pt,u1,v1,p0,p1,p2));
					glColorPT(hiperb(w,cl,u1,v0,p0,p1,p2));
					glVertexPT(hiperb(w,pt,u1,v0,p0,p1,p2));
					glColorPT(hiperb(w,cl,u0,v1,p0,p1,p2));
					glVertexPT(hiperb(w,pt,u0,v1,p0,p1,p2));
				}
				
			}
		}
		glEnd();
		
		
		
	} else { // bilineal
		
		
		glBegin(GL_QUADS);
		for(int i=0;i<n;i++) { 
			float u0=1.f/(n)*i;
			float u1=1.f/(n)*(i+1);
			for(int j=0;j<n;j++) { 
				float v0=1.f/(n)*j;
				float v1=1.f/(n)*(j+1);
				glColorPT(bilineal(cl,u0,v0));
				glVertexPT(bilineal(pt,u0,v0));
				glColorPT(bilineal(cl,u1,v0));
				glVertexPT(bilineal(pt,u1,v0));
				glColorPT(bilineal(cl,u1,v1));
				glVertexPT(bilineal(pt,u1,v1));
				glColorPT(bilineal(cl,u0,v1));
				glVertexPT(bilineal(pt,u0,v1));
			}
		}
		glEnd();
	}
	glPointSize(7);
	glBegin(GL_POINTS);
	for(int i=0;i<4;i++) {
		glColorPT(cl[i]);
		glVertexPT(pt[i]);
		
	}
	glEnd();
  if (diag) {
		glLineWidth(3);
		glColor3i(0,0,0);
		if (metodo==MDiagVA || metodo==MDiagRR) {
			glBegin(GL_LINES);
			if (metodo==0) {
				glVertexPT(pt[1]);
				glVertexPT(pt[3]);
				Punto r = lineal(pt,.5,.5,1,3,0);
				glVertexPT(pt[0]);
				glVertexPT(r);
				glVertexPT(r);
				glVertexPT(pt[2]);
			} else {
				glVertexPT(pt[0]);
				glVertexPT(pt[2]);
        Punto r = lineal(pt,.5,.5,0,2,3);
				glVertexPT(pt[1]);
  			glVertexPT(r);
				glVertexPT(r);
				glVertexPT(pt[3]);
			}
			glEnd();
		} else if (metodo==2) {
			glBegin(GL_LINE_STRIP);
			for(int i=0;i<=n;i++) {
				float a=1.f/(n)*i;
				glVertexPT(bilineal(pt,a,a));
			}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for(int i=0;i<=n;i++) {
				float a=1.f/(n)*i;
				glVertexPT(bilineal(pt,1-a,a));
			}
			glEnd();
		} else {
			glBegin(GL_LINES);
			glVertexPT(pt[1]);
			glVertexPT(pt[3]);
			glVertexPT(pt[0]);
			glVertexPT(c);
			glVertexPT(c);
			glVertexPT(pt[2]);
			glEnd();
		}
		glLineWidth(1);
	}
  
  if (help) {
		glClear(GL_COLOR_BUFFER_BIT);
		for(int i=0;i<4;i++) 
			OSD<<"Tecla " << i+1 << ": metodo " << smetodo[i] << '\n';
		OSD<<"Tecla M: Ciclar entre los metodos de interpolacion\n";
		OSD<<"Tecla F: Alternar entre relleno y alambre\n";
		OSD<<"Tecla D: Mostrar/Ocultar diagonales\n";
		OSD<<"Teclas -/+: Bajar/Subir nivel de detalle\n";
		OSD<<"\n";
		OSD<<"Arrastrar con el raton los vertices para\n";
		OSD<<"deformar el cuadrilatero\n";
		OSD<<"\n";
		OSD<<"Presione H para ocultar esta ayuda";
		
	} else {
		OSD<<"Modo: " << smetodo[metodo] << "\n";
		OSD<<"(presione H para ver la ayuda)";
	}
	glColor3f(0,0,0);
	OSD.Render(win_w,win_h);
  
	glutSwapBuffers();
}

void motion_cb(int x, int y) {
	if (sel==-1) return;
	pt[sel].x=x;
	pt[sel].y=win_h-y;
	glutPostRedisplay();
}
void mouse_cb(int button, int state, int x, int y) {
	y=win_h-y;
	if (state==GLUT_DOWN) {
		float mind;
		for(int i=0;i<4;i++) { 
			float d=(pt[i]-Punto(x,y)).Mod2();
			if (i==0||d<mind) { sel=i; mind=d; }
		}
	}
}

void keyboard_cb(unsigned char key, int x, int y) {
	const int cant_modos = 4;
	switch (key) {
	case 'm': metodo=(metodo+1)%cant_modos; break;
	case 'M': metodo=(metodo+cant_modos-1)%cant_modos; break;
	case 'f': case 'F': filled=!filled; break;
	case 'd': case 'D': diag=!diag; break;
	case 'h': case 'H': help=!help; break;
	case '-': if(n>1) n--; break;
	case '+':n++; break;
	default:
		if (key>'0'&&key<='0'+cant_modos) metodo = key-'1';
	}
	glutPostRedisplay();
}

void initialize() {
	glutInitDisplayMode (GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize (win_w,win_h);
	glutInitWindowPosition (100,100);
	glutCreateWindow ("Interpolación Bilineal");
	glutDisplayFunc (display_cb);
	glutReshapeFunc (reshape_cb);
	glutMouseFunc(mouse_cb);
	glutMotionFunc(motion_cb);
	glutKeyboardFunc(keyboard_cb);
	glClearColor(1.f,1.f,1.f,1.f);
}

int main (int argc, char **argv) {
	glutInit (&argc, argv);
	initialize();
	glutMainLoop();
	return 0;
}

