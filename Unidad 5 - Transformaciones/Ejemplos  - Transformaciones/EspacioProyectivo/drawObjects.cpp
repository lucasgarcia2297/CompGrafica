#ifndef drawObjects_H
#define drawObjects_H
#include <cstdio>
#include <cmath>
#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <OpenGL/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif
#include "globals.hpp"
#include "../common/OSD.hpp"
#include <ctime>
using namespace std;

static const int gpp=20; // cantidad de puntos que definen el "circulo" del plano verde
static float green_plane[gpp][3];
static float zero[]={0,0,0,0};

void mk_green_plane() {
	srand(time(NULL));
	double a,cosa,sina,r;
	for (int i=0;i<gpp;i++) {
		a=i*2*M_PI/gpp;
		sina=sin(a);
		cosa=cos(a);
		r=4+rand()%3;
		green_plane[i][0]=r*cosa;
		green_plane[i][1]=r*sina;
		green_plane[i][2]=1;
	}
}

// acomoda las matrices para dibujar las cosas en 3D
void set_3d_matrix() { 
	glViewport (0, 0, win_w, win_h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	if (perspective) gluPerspective (100, win_w / win_h, 1.0, 100.0);
	else glOrtho(-d,d,-d,d,-100,100);
	glMatrixMode (GL_MODELVIEW);
}

// acomoda las matrices para dibujar los textos con coord de la ventana 2D
void set_2d_matrix() {
	glViewport(0, 0, win_w, win_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_w, win_h, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// coloca los rotulos de los ejes
static void *font=GLUT_BITMAP_HELVETICA_18; // fuente para rotulos, ayuda y matriz
void draw_axis_labels() {
	
	glPushMatrix();
	glTranslatef(0.075,0.075,0.075);
	glColor3f(0,0,0);
	
	glRasterPos3f(1,0,0);
	glutBitmapCharacter(font,'x');
	
	glRasterPos3f(0,1,0);
	glutBitmapCharacter(font,'y');
	
	glRasterPos3f(0,0,1);
	glutBitmapCharacter(font,'w');
	glutBitmapCharacter(font,'=');
	glutBitmapCharacter(font,'1');
	
	glRasterPos3f(0,0,0);
	glutBitmapCharacter(font,'o');
	
	glPopMatrix();
	
}


// dibuja el triangulo proyectado sobre w=1
void draw_t_down() {
	glColor4f(.9,.9,.5,.5);
	glBegin(quad?GL_QUADS:GL_TRIANGLES);
	glVertex4fv(pts_trans[0]);
	glVertex4fv(pts_trans[1]);
	glVertex4fv(pts_trans[2]);
	if (quad)  glVertex4fv(pts_trans[3]);
	glVertex4fv(pts_trans[4]);
	glVertex4fv(pts_trans[5]);
	glVertex4fv(pts_trans[6]);
	if (quad)  glVertex4fv(pts_trans[7]);
	glEnd();
	
	glEnable(GL_POLYGON_OFFSET_LINE); glPolygonOffset (1,-1);
	glColor4f(.9,.9,0,1);
	glBegin(GL_LINE_LOOP);
	glVertex4fv(pts_trans[0]);
	glVertex4fv(pts_trans[1]);
	glVertex4fv(pts_trans[2]);
	if (quad) glVertex4fv(pts_trans[3]);
	glEnd();
	glLineWidth(3);
	glBegin(GL_LINE_LOOP);
	glVertex4fv(pts_trans[4]);
	glVertex4fv(pts_trans[5]);
	glVertex4fv(pts_trans[6]);
	if (quad) glVertex4fv(pts_trans[7]);
	glEnd();
	glDisable(GL_POLYGON_OFFSET_LINE); 
}

// dibuja el triangulo transformado
void draw_t_up() {
	glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1);
	glColor4f(1,.5,.5,.5);
	glBegin(quad?GL_QUADS:GL_TRIANGLES);
	glVertex3fv(pts_trans[0]);
	glVertex3fv(pts_trans[1]);
	glVertex3fv(pts_trans[2]);
	if (quad) glVertex3fv(pts_trans[3]);
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	glColor4f(1,0,0,1);
	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex3fv(pts_trans[0]);
	glVertex3fv(pts_trans[1]);
	glVertex3fv(pts_trans[1]);
	glVertex3fv(pts_trans[2]);
	glVertex3fv(pts_trans[2]);
	if (quad) {
		glVertex3fv(pts_trans[3]);
		glVertex3fv(pts_trans[3]);
	}
	glVertex3fv(pts_trans[0]);
	glEnd();
}

// dibuja las lineas del origen a los vertices del triangulo
void draw_t_plines() {
	
	glColor4f(1,0,1,1);
	glBegin(GL_LINES);
	glLineWidth(3);
	for(int j=0;j<4;j++) { 
		if (!quad && j==3) continue;
		glVertex3fv(zero);
		if (pts_trans[j][2]>=1 || pts_trans[j][2]<0)
			glVertex3fv(pts_trans[j]);
		else
			glVertex4fv(pts_trans[j]);
	}
	for(int j=4;j<8;j++) {
		if (!quad && j==7) continue;
		if (pts_trans[j][2]>0)
		{ glVertex3fv(zero); glVertex4fv(pts_trans[j]); }
	}
	glEnd();
	
	if (show_full_lines) {
		glLineWidth(1);
		glBegin(GL_LINES);
		for(int j=4;j<8;j++) {
			if (!quad && j==7) continue;
			float aux[4] = {0};
			for(int k=0;k<3;k++) aux[k]=pts_trans[j][k];
			glVertex4fv(aux);
			glVertex3fv(zero);
			for(int k=0;k<3;k++) aux[k]=-aux[k];
			glVertex4fv(aux);
			glVertex3fv(zero);
		}
		glEnd();
	}
	
	
}

// dibuja el plano w=1
void draw_w1_plane() {
	
	glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1);
	glColor4f(0,1,0,.6);
	glBegin(GL_TRIANGLES);
	for (int i=0;i<gpp;i++) {
		glVertex3f(0,0,1);
		glVertex3fv(green_plane[i]);
		glVertex3fv(green_plane[(i+1)%gpp]);
	}
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
	
}

void draw_transf_axis() {
	glLineWidth(3);
	glBegin(GL_LINES);
	for(int k=0;k<3;k++) { 
		glColor3f(k==0?1:0, k==1?1:0, k==2?1:0);
		glVertex3fv(zero);
		glVertex3f(mat[0][k],mat[1][k],mat[2][k]);
	}
	glEnd();
}

void draw_bb_lines() {
	glLineWidth(1);
	glColor3f(0.4,0.4,0.4);
	glBegin(GL_LINES);
	glVertex3fv(pts_bb[0]); glVertex3fv(pts_bb[1]);
	glVertex3fv(pts_bb[1]); glVertex3fv(pts_bb[2]);
	glVertex3fv(pts_bb[2]); glVertex3fv(pts_bb[3]);
	glVertex3fv(pts_bb[3]); glVertex3fv(pts_bb[0]);
	glVertex3fv(pts_bb[4]); glVertex3fv(pts_bb[5]);
	glVertex3fv(pts_bb[5]); glVertex3fv(pts_bb[6]);
	glVertex3fv(pts_bb[6]); glVertex3fv(pts_bb[7]);
	glVertex3fv(pts_bb[7]); glVertex3fv(pts_bb[4]);
	glVertex3fv(pts_bb[0]); glVertex3fv(pts_bb[4]);
	glVertex3fv(pts_bb[1]); glVertex3fv(pts_bb[5]);
	glVertex3fv(pts_bb[2]); glVertex3fv(pts_bb[6]);
	glVertex3fv(pts_bb[3]); glVertex3fv(pts_bb[7]);
	glEnd();
}

// dibuja las lineas de los ejes
void draw_olines() {
	glLineWidth(show_ejes?1:3);
	
	glColor3f(0,0,.8);
	glBegin(GL_LINES);
	glVertex3f(-100,0,0);
	glVertex3f(+100,0,0);
	glVertex3f(0,-100,0);
	glVertex3f(0,+100,0);
	glColor3f(0,0,0);
	glVertex3f(0,0,-100);
	glVertex3f(0,0,+100);
	glEnd();
	
	glColor3f(0,0,0);
	glPointSize(4);
	glBegin(GL_POINTS);
	glVertex3f(0,0,0);
	glEnd();
}

void draw_w1_axis() {
	glLineWidth(show_ejes?1:3);
	
	glColor3f(0,.7,0);
	glBegin(GL_LINES);
	glVertex3f(-100,0,1);
	glVertex3f(+100,0,1);
	glVertex3f(0,-100,1);
	glVertex3f(0,+100,1);
	glEnd();
	
	glColor3f(0,.5,0);
	glPointSize(4);
	glBegin(GL_POINTS);
	glVertex3f(0,0,1);
	glEnd();
}

// dibuja el plano w=0
void draw_w0_plane() {
	if (!show_w0_plano) return;
	glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1);
	glColor4f(0,0,1,.5);
	glBegin(GL_QUADS);
	glVertex3f(-100,-100,0);
	glVertex3f(100,-100,0);
	glVertex3f(100,100,0);
	glVertex3f(-100,100,0);
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void drawObjects() {
	
	set_3d_matrix();
		
	glClearColor (1,1,1,1);
	glEnable(GL_DEPTH_TEST);
	glLoadIdentity(); 
	double cy=-d*cos(a1)*cos(a2), cx=-d*sin(a1)*cos(a2), cz=d*sin(a2);
	gluLookAt(cx,cy,cz, 0.0, 0.0, 1, 0.0, 0.0, a2>=M_PI/2?-1.0:1.0);
	
	if (show_axis_label) draw_axis_labels();
	if (show_bounding_box) draw_bb_lines();
	draw_olines();
	if (show_w1_ejes) draw_w1_axis();
	if (show_ejes) draw_transf_axis();
	
	// el orden de dibujado cambia para que quede bien la combinacion transparencia+zbuffer
//	if (2*tw*ts>1) {
		if (a2>.2) {
			draw_t_plines();
			draw_w0_plane();
			glDepthMask(false);
			draw_t_up();
			glDepthMask(true);
			draw_t_down();
			draw_t_up();
			if (show_w1_plano) draw_w1_plane();
		} else {
			draw_t_plines();
			draw_t_up();
			if (show_w1_plano) draw_w1_plane();
			draw_t_down();
			draw_w0_plane();
		}
//	} else {	
//		if (a2>.2) {
//			draw_t_plines();
//			draw_w0_plane();
//			draw_t_up();
//			if (show_w1) draw_w1_plane();
//			draw_t_down();
//		} else {
//			draw_t_plines();
//			if (show_w1) draw_w1_plane();
//			draw_t_up();
//			draw_t_down();
//			draw_w0_plane();
//		}
//	}
	// texto
	glColor3f(0,0,0);
	if (show_help) {
		OSD << "Demo Espacio Proyectivo P2\n";
		OSD << "  Por Pablo Novara\n";
		OSD << "  Version " << VERSION << '\n';
		OSD << '\n';
		OSD << "Teclas:";
		OSD << '\n';
		OSD << "  d: Drag=Shear en X e Y\n";
		OSD << "  r: Drag=Rotación respecto a W\n";
		OSD << "  t: Drag=Rotación respecto a X\n";
		OSD << "  u: Drag=Escalado uniforme\n";
		OSD << "  s: Drag=Escalar el eje W\n";
		OSD << "  x: Drag=Escalar el plano XY\n";
		OSD << "  z: Resetear transformaciones\n";
		OSD << '\n';
		OSD << "  h: Muestra/oculta esta ayuda\n";
		OSD << "  e: Muestra/oculta los ejes transformados\n";
		OSD << "  l: Muestra/oculta las rectas completas\n";
		OSD << "  i: Muestra/oculta plano ideal W=0\n";
		OSD << "  w: Muestra/oculta plano W=1\n";
		OSD << "  1: Muestra/oculta los ejes XY en W=1\n";
		OSD << "  a: Muestra/oculta etiquetas\n";
		OSD << "  b: Muestra/oculta bounding-box\n";
		OSD << "  q: Alterna triangulo/cuadrilatero\n";
		OSD << "  p: Alterna proyección ortogonal/perspectiva\n";
		OSD << "  f: Pantalla completa\n";
		OSD << "  Esc: Exit\n";
	} else {
		// muestra la matriz 
		for(int k=0;k<3;k++) {
			for(int j=0;j<3;j++)
				OSD << (mat[k][j]>=0?"   ":"  ") << mat[k][j]; 
			OSD << '\n';
		}
	}
	OSD << '\n';
	OSD << 	help_texts[def_drag_mode];
	if (!show_help) OSD <<  "     (presione H para ver mas opciones)";
	OSD.Render(win_w,win_h,true);
}

#endif

