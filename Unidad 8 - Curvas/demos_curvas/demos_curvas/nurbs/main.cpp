#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#ifdef __APPLE__
# include <OpenGL/gl.win_h>
# include <GLUT/glut.win_h>
#else
# include <GL/gl.h>
# include <GL/glut.h>
#endif
#include "OSD.hpp"
#include "nurbs.h"
#include "render.h"

#define SVERSION "20191029"

const char *help_text= 
	" Zaskar's NURBS Demo (v" SVERSION ", by Pablo Novara)\n"
	"==================================================\n\n"
	"Ayuda rapida: \n\n"
	"  click izquierdo en el aire: AGREGAR PUNTO DE CONTROL\n"
	"  arrastrar con click izquierdo: MOVER KNOT O PTO DE CONTROL\n"
	"  X o click izquierdo en la de knots: AGREGAR UN KNOT\n"
	"  click derecho en el aire: MOVER toda la CURVA\n"
	"  arrastrar con click derecho: CAMBIAR coordenada W DE UN PTO de control\n"
	"  rueda del mouse/teclas + y -: ZOOM IN/OUT en torno al cursor\n"
	"  1 a 9: CAMBIAR GRADO de la nurbs\n"
	"  DEL: QUITAR PTO DE CONTROL seleccionado\n"
	"  C: BORRAR toda LA CURVA\n"
	"  W: SETEAR W=1 en el pto de control seleccionado\n"
	"  I: reacomodar knots para INTERPOLAR EL PTO de control selec.\n"
	"  O: ALINEAR los pto de control anterior y siguiente con el actual.\n"
	"  R: INVERTIR EL SENTIDO de la curva\n"
	"  Q: seleccionar solo con CLICK\n"
	"  S: GUARDAR la nurbs (en el archivo que se paso por parametro al ejecutable)\n"
	"  A: ANIMAR punto sobre la curva\n"
	"  Redistribuir Knots:\n"
	"      N:EQUIESPACIADOS    m:EXTREMOS BEZIER    b:SPLINE BEZIER\n"
	"  Mostrar/Ocultar:\n"
	"      P: PUNTOS DE CTRL      G: POLIGONO DE CTRL         L: VECTOR DE KNOTS\n"
	"      T: NIVEL DE DETALLE    D: DECASTELJAU/BLOOSOMING   H:AYUDA\n"
	"      K: KNOTS EN LA CURVA   F: FUNC. BASE               U: TRAMOS de la curva\n"
	;


extern const int MAX_DETAIL=499;
extern const int MARGIN=15;

enum {
	MT_NONE,
	MT_CONTROL,
	MT_COORD_W,
	MT_KNOT,
	MT_DETAIL,
	MT_MOVE
};

int win_w=800,win_h=600;

float sel_x,sel_y;
int sel_control=SEL_NONE, sel_knot=SEL_NONE;
bool sel_detail;
float sel_u=SEL_NONE;
int detail_line=0;
int drag=MT_NONE;
int last_mx, last_my;
int detail_pos=25;
int offset_x=0, offset_y=0;
float last_w, last_ox, last_oy;
int knot_line = 0;
std::string nurb_file;
int decasteljau_level = 0;
int animate_speed = 5;

// para indicarle a update_selections que se movio el mouse para que marque la nueva seleccion
bool mouse_moved=false;
bool freeze_selections=false;
int mx=0,my=0;

bool draw_basis=false;
bool draw_knots=false;
bool draw_polygon=true;
bool draw_control=true;
bool draw_detail=false;
bool draw_kline=true;
bool draw_w1=true;
bool draw_parts=false;
bool animate_t=false;

int knots_nodes[ARRAY_MAX];

NURBS nurbs;


// este ya no es realmente el idle callback, porque si lo pongo como idle
// callback frie la cpu aunque no hagamos nada... así que lo llamo yo desde el 
// passive_motion... probablemente también debería llamarlo desde otros lados
static void update_selections() {
	int x=mx, y=my;
	if (!mouse_moved || drag!=MT_NONE) return; // si no estamos haciendo nada marcar la seleccion si hay algo debajo del cursor
	if (!animate_t) sel_control=sel_knot=SEL_NONE;
	
	// linea que controla el nivel de detalle del rasterizado
	if (draw_detail && x>win_w-MARGIN-10 && x<win_w-MARGIN+10 && y>detail_pos-10 && y<detail_pos+10) {
		sel_detail=true;
		OSD<<"tol = "<<nurbs.detail << '\n';
		glutPostRedisplay();
		
	} else {
		if (sel_detail) {
			OSD.Clear();
			sel_detail=false;
		}
		
		// knots/u sobre la linea de knots
		if (!animate_t && draw_kline && y<win_h-MARGIN+10 && y>win_h-MARGIN-10) { // seleccion en la linea de los knots
			int min_d=5, m_ksel=SEL_NONE;
			for (int i=0;i<nurbs.knum;i++) { 
				int d1 = knots_nodes[i]>x? knots_nodes[i]-x : x-knots_nodes[i]; 
				if (d1<min_d) {
					min_d=d1;
					m_ksel=i;
				}
			}
			if (m_ksel!=SEL_NONE) { // si esta repetido, buscar el de "afuera"
				if (x>knots_nodes[m_ksel]) {
					while (m_ksel+1<nurbs.knum && knots_nodes[m_ksel]==knots_nodes[m_ksel+1])
						m_ksel++;
				} else {
					while (m_ksel>0 && knots_nodes[m_ksel]==knots_nodes[m_ksel-1])
						m_ksel--;
				}
				
				if (sel_knot!=m_ksel) {
					sel_knot = m_ksel;
					sel_u = nurbs.knots[m_ksel];
					OSD << "knot " << m_ksel << "  -  u = " << nurbs.knots[m_ksel] << '\n';
				}
				
			} else if (x>=MARGIN && x<=win_w-MARGIN) { // si no hay ninguno marcar la posicion para agregar
				if (x!=sel_x || sel_knot!=SEL_NEW) {
					sel_knot=SEL_NEW; sel_u = float(sel_x-MARGIN)/knot_line;
					OSD << "insertar knot  -  u = " << sel_u << '\n';
					sel_x=x;
				}
			} else if (sel_control!=SEL_NONE || sel_knot!=SEL_NONE) {
				sel_control=sel_knot==SEL_NONE;
				OSD.Clear();
			}
		} 
		
		// puntos de control y knots sobre la curva
		else if (draw_control){ 
			
			// punto de control
			int min_d=10, m_csel=SEL_NONE;
			for (int i=0;i<nurbs.num;i++) {
				float wi=nurbs.controls[i][W],xi=nurbs.controls[i][X]/wi,yi=nurbs.controls[i][Y]/wi;
				int d1 = int(xi<x?x-xi:xi-x);
				int d2 = int(yi<y?y-yi:yi-y);
				if (d1<min_d && d2<min_d) {
					m_csel=i;
					min_d = d1>d2?d2:d1;
				}
			}
			if (m_csel!=sel_control) { // si cambio la seleccion, actualizar
				sel_control=m_csel;
				sel_knot=SEL_NONE;
				if (sel_control==SEL_NONE)
					OSD.Clear();
				else
					OSD << "pto control " << m_csel << "  -  win_w = " << nurbs.controls[m_csel][W] << '\n';
			} 
			
			// knots sobre la curva
			if (!animate_t && m_csel==SEL_NONE) {
				int min_dist=5;
				for (int i=nurbs.order;i<nurbs.knum;i++) {
					int d1 = nurbs.knots_points[i][X]-x;
					int d2 = nurbs.knots_points[i][Y]-y;
					if (d1>=-min_dist && d1<=min_dist && d2>=-min_dist && d2<=min_dist) {
						sel_knot=i;
						if (d1<0)
							d1=-d1;
						if (d2<0)
							d2=-d2;
						min_dist = d1<d2?d1:d2;
					}
				}
			}
		}
		
		// selección de un u sobre la curva
		if (sel_control==SEL_NONE && sel_knot==SEL_NONE) {
			sel_u=nurbs.FindU(x,y);
			if (sel_u!=SEL_NONE) {
				sel_knot=SEL_NEW;
				sel_x=MARGIN+knot_line*sel_u;
				OSD << "u = " << sel_u << '\n';
				sel_control=SEL_NONE;
			}
		}
		
		glutPostRedisplay();
	}
	mouse_moved=false;
}

// callback del motion de la ventana de imagenes
static void motion_cb(int x, int y) { 
	mx=x;my=y;
	mouse_moved=true;
	if (drag==MT_CONTROL) { // si se esta arrastrando un punto de control
		nurbs.controls[sel_control][X]=x*nurbs.controls[sel_control][W];
		nurbs.controls[sel_control][Y]=y*nurbs.controls[sel_control][W];
		glutPostRedisplay();
	} else if (drag==MT_COORD_W) { // si se esta cambiando la win_w de un pto de control
		float oldW=nurbs.controls[sel_control][W], newW=last_w+float(last_my-y)/20;
		if (newW==0) newW=1e-10;
		nurbs.controls[sel_control][X]*=newW/oldW;
		nurbs.controls[sel_control][Y]*=newW/oldW;
		nurbs.controls[sel_control][W]=newW;
		glutPostRedisplay();
		OSD << "pto control " << sel_control << "  -  win_w = " << nurbs.controls[sel_control][W] << '\n';
	} else if (drag==MT_KNOT) { // si se esta arrastrando un knot
		float p=float(x-MARGIN+sel_x)/(knot_line);
		if (p<0) p=0;
		else if (p>1) p=1;
		if (sel_knot>0 && nurbs.knots[sel_knot-1]>p)
			nurbs.knots[sel_knot]=nurbs.knots[sel_knot-1];
		else if (sel_knot+1<nurbs.knum && nurbs.knots[sel_knot+1]<p)
			nurbs.knots[sel_knot]=nurbs.knots[sel_knot+1];
		else
			nurbs.knots[sel_knot]=p;
		glutPostRedisplay();
		OSD << "knot " << sel_knot << "  -  u = " << nurbs.knots[sel_knot] << '\n';
	} else if (drag==MT_DETAIL) { // si se esta cambiando la tolerancia para el dibujo
		float d = float(y-MARGIN)/detail_line;
		if (d>=0) {
			nurbs.detail = std::pow(d,3)*MAX_DETAIL;
			detail_pos = y;
		}
		OSD << "tol = " << nurbs.detail+1 << '\n';
		glutPostRedisplay();
	} else if (drag==MT_MOVE) {
		offset_x = int(last_ox+(x-last_mx));
		offset_y = int(last_oy+(y-last_my));
		glutPostRedisplay();
	} else {
		if (!freeze_selections) update_selections();
	}
}

// callback del mouse de la ventana de imagenes
static void mouse_cb(int button, int state, int x, int y){
	if (button==GLUT_LEFT_BUTTON) {
		if (state==GLUT_DOWN) {
			if (freeze_selections) { update_selections(); return; }
			if (drag==MT_NONE) {
				if (sel_detail)
					drag = MT_DETAIL;
				else if (sel_control!=SEL_NONE) { 
					if (sel_control!=SEL_NEW) // mover un punto de control
						drag = MT_CONTROL;
				} else if (sel_knot!=SEL_NONE) { 
					if (sel_knot!=SEL_NEW) { // mover un knot existente
						drag = MT_KNOT;
						sel_x=MARGIN+knot_line*nurbs.knots[sel_knot]-x;
					} else if (sel_knot==SEL_NEW) { // insertar un nuevo knot
						drag = MT_KNOT;
						sel_knot = nurbs.InsertKnot(float(sel_x-MARGIN)/knot_line);
						sel_x=MARGIN+knot_line*nurbs.knots[sel_knot]-x;
						glutPostRedisplay();
					}
				} else { // agregar un punto de control
					OSD << "pto control " << nurbs.num << '\n';
					drag=MT_CONTROL;
					sel_control = nurbs.AddControlPoint(x,y);
					glutPostRedisplay();
				}
			}
		} else {
			drag = MT_NONE;
			motion_cb(x,y);
		}
	} else 	if (button==3) {
		nurbs.ZoomIn(x,y);
		glutPostRedisplay();
	} else 	if (button==4) {
		nurbs.ZoomOut(x,y);
		glutPostRedisplay();
	} else 	if (button==GLUT_RIGHT_BUTTON) {
		if (state==GLUT_DOWN) {
			if (sel_control!=SEL_NONE) {
				last_my = y;
				last_w = nurbs.controls[sel_control][W];
				drag = MT_COORD_W;
			} else {
				sel_control=sel_knot=SEL_NONE;
				sel_detail=false;
				last_mx=x; last_my=y;
				last_ox = offset_x;
				last_oy = offset_y;
				drag=MT_MOVE;
			}
		} else {
			if (drag==MT_COORD_W || drag==MT_MOVE)
				drag = MT_NONE;
		}
	}

}

static void reshape_cb(int aw, int ah){
	win_w=aw;win_h=ah;

	if (!win_h||!win_w) return;

	glViewport(0,0,win_w,win_h); // region donde se dibuja
	knot_line=win_w-30;
	detail_line=win_h-MARGIN*3;
	detail_pos = int(MARGIN+pow((nurbs.detail-1)/MAX_DETAIL,1.0/3.0)*detail_line);

	// matriz de proyeccion
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,win_w,win_h,0,-1,1);

	glutPostRedisplay();
}

static void idle_cb() {
	static int anterior = glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
	int tiempo = glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
	constexpr int msecs_per_frame=20; 
	if (lapso<msecs_per_frame) return;
	anterior = tiempo;
	sel_x += animate_speed; if (sel_x>win_w) sel_x=0;
	sel_knot = SEL_NEW;
	glutPostRedisplay();
}

// callback del teclado
static void keyboard_cb(unsigned char key,int x=0,int y=0) {
	switch(tolower(key)) {
	case 27:
		exit(0);
	case 'h':
		OSD<<help_text<<'\n';
		break;
	case 'k':
		draw_knots=!draw_knots;
		sel_u=sel_knot=SEL_NONE;
		break;
	case 'd':
		decasteljau_level++;
		break;
	case 'q':
		freeze_selections=!freeze_selections;
		break;
	case 't':
		draw_detail=!draw_detail;
		sel_detail=false;
		break;
	case 'l':
		draw_kline=!draw_kline;
		break;
	case 'f':
		draw_basis=!draw_basis;
		break;
	case 'g':
		draw_polygon=!draw_polygon;
		break;
	case 'p':
		draw_control=!draw_control;
		sel_u=sel_control=SEL_NONE;
		break;
//	case 'w':
//		draw_w1=!draw_w1;
//		break;
	case 'c':
		sel_u=sel_knot=sel_control=SEL_NONE;
		nurbs.Clear();
		break;
	case 127: 
		if (sel_control!=SEL_NONE) {
			nurbs.DeleteControl(sel_control);
			sel_control=SEL_NONE;
		}
		break;
	case 'w':
		if (sel_control!=SEL_NONE) {
			GLfloat &win_w = nurbs.controls[sel_control][W];
			nurbs.controls[sel_control][X]/=win_w;
			nurbs.controls[sel_control][Y]/=win_w;
			win_w=1;
			OSD << "pto control " << sel_control << "  -  win_w = 1" << '\n';
		}
		break;
	case 'o':
		if (sel_control!=SEL_NONE && sel_control>0 && sel_control<nurbs.num-1) {
			int i = sel_control;
			double va[2], vp[2], v[2];
			v[0] = nurbs.controls[i+1][X] - nurbs.controls[i-1][X];
			v[1] = nurbs.controls[i+1][Y] - nurbs.controls[i-1][Y];
			va[0] = nurbs.controls[i][X] - nurbs.controls[i-1][X];
			va[1] = nurbs.controls[i][Y] - nurbs.controls[i-1][Y];
			vp[0] = nurbs.controls[i+1][X] - nurbs.controls[i][X];
			vp[1] = nurbs.controls[i+1][Y] - nurbs.controls[i][Y];
			float mv = sqrt(v[0]*v[0]+v[1]*v[1]);
			float ma = sqrt(va[0]*va[0]+va[1]*va[1]);
			float mp = sqrt(vp[0]*vp[0]+vp[1]*vp[1]);
			va[0] = v[0]/mv*ma; va[1] = v[1]/mv*ma;
			vp[0] = v[0]/mv*mp; vp[1] = v[1]/mv*mp;
			
			nurbs.controls[i-1][X] = nurbs.controls[i][X] - va[0];
			nurbs.controls[i-1][Y] = nurbs.controls[i][Y] - va[1];
			nurbs.controls[i+1][X] = nurbs.controls[i][X] + vp[0];
			nurbs.controls[i+1][Y] = nurbs.controls[i][Y] + vp[1];
		}
		break;
	case 'i':
		if (sel_control!=SEL_NONE) nurbs.Interpolate(sel_control);
		else if (sel_u!=SEL_NONE) nurbs.InsertKnot(sel_u,true);
		break;
	case '1':case '2':case '3': case '4': case '5': case '6': case'7': case '8': case '9':
		nurbs.SetDegree(key-'0');
		sel_u=sel_knot=sel_control=SEL_NONE;
		OSD << "grado = " << int(key-'0') << '\n';
		break;
	case 's':
		nurbs.Save(nurb_file);
		OSD << "nurbs guardada: " << nurb_file << '\n';
		break;
	case 'n':
		sel_knot=sel_control=SEL_NONE;
		sel_u=SEL_NONE;
		nurbs.ResetKnots(KD_UNIFORM);
		break;
	case 'm':
		sel_u=sel_knot=sel_control=SEL_NONE;
		nurbs.ResetKnots(KD_BEZIER_BOUNDARY);
		break;
	case 'b':
		sel_u=sel_knot=sel_control=SEL_NONE;
		nurbs.ResetKnots(KD_BEZIER_SPLINE);
		break;
	case 'x':
		if (sel_knot!=SEL_NONE) {
			float pos = sel_knot==SEL_NEW? float(sel_x-MARGIN)/knot_line : nurbs.knots[sel_knot];
			sel_knot = nurbs.InsertKnot(pos);
		}
		break;
	case 'r':
		nurbs.InvertCurve();
		sel_u=sel_knot=sel_control=SEL_NONE;
		break;
	case '+':
		if (animate_t) {
			animate_speed++;
		} else {
			sel_control=SEL_NONE;
			sel_u=SEL_NONE;
			nurbs.ZoomIn(x,y);
		}
		break;
	case 'a':
		animate_t = !animate_t;
		glutIdleFunc(animate_t?idle_cb:nullptr);
		break;
	case 'u':
		draw_parts = !draw_parts;
		break;
	case '-':
		if (animate_t) {
			if (animate_speed>1) animate_speed--;
		} else {
			sel_control=SEL_NONE;
			sel_u=SEL_NONE;
			nurbs.ZoomOut(x,y);
		}
		break;
	default:
		;
	}
	glutPostRedisplay();
}

static void init() {
	
	OSD.SetMode(OSD_t::PersistentMode);
	OSD<<"(presione H para ver la ayuda)"<<'\n';
	
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
	
	glutInitWindowSize(win_w,win_h); 
	glutInitWindowPosition(100,100);
	glutCreateWindow("NURBS Demo");
	glMatrixMode(GL_PROJECTION); 
	glOrtho(0,win_w,0,win_h,1,1);
	
	glutDisplayFunc(display_cb);
	glutMouseFunc(mouse_cb);
	glutKeyboardFunc(keyboard_cb);
	glutMotionFunc(motion_cb);
	glutPassiveMotionFunc(motion_cb);
	glutReshapeFunc(reshape_cb);
	
	knot_line=win_w-MARGIN*2;
	detail_line=win_h-MARGIN*3;
	detail_pos = int(MARGIN+pow((nurbs.detail-1)/MAX_DETAIL,1.0/3.0)*detail_line);
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
	
}

int main(int argc, char *argv[]){
	glutInit(&argc,argv); // inicializa glut
	init();
	if (argc==1) {
		nurb_file = "nurbs.nurbs";
	} else {
		nurb_file = argv[1];
		nurbs.Load(nurb_file);
	}
	glutMainLoop(); 
	return 0;
}
