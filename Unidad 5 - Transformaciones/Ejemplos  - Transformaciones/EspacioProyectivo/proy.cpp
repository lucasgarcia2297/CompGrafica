#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <OpenGL/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif
#include <cstdlib>
#include <iostream>
#include <cmath>
#include "globals.hpp"

static int lmx,lmy, lww, lwh; // tamanio y posicion de la ventana antes de pasar a pantalla completa
static bool fullscreen=false; // si esta a pantalla completa

void drawObjects();

// dibuja todo, llama a las funciones anteriores en el orden adecuado
// para evitar los problemas de transparencia+zbuffer
void display_cb (void) {

	update_points_and_matrix();
	
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	drawObjects();
	
	glutSwapBuffers();
	
}

// callaback para el cambio de tamanio de la ventana
void reshape_cb (int w, int h) {
	win_w=w; win_h=h;
//	if (w&&h) set_3d_matrix();
	display_cb();
}

// callback para el arrastre del mouse, mueve la "camara" en circulos
void motion_cb (int x, int y) {
	switch (cur_drag_mode) {
		
	case DM_CAMERA:
		y=win_h-y;
		a2+=0.003*(lmy-y);
		a1-=0.003*(lmx-x);
		lmx=x; lmy=y;
		break;
	
	case DM_ROTATE_X:
		y=win_h-y;
		tp+=sin(a1+tr)*(x-lmx)/200;
		tp+=cos(a1+tr)*(lmy-y)/200;
		lmx=x; lmy=y;
		break;
		
	case DM_MOVE_XY:
		y=win_h-y;
		tx+=cos(a1+tr)*(x-lmx)/200;
		tx+=sin(a1+tr)*(y-lmy)/200;
		ty+=sin(a1+tr)*(lmx-x)/200;
		ty+=cos(a1+tr)*(y-lmy)/200;
		lmx=x; lmy=y;
		break;
		
	case DM_SCALE_W:
		y=win_h-y;
		tw+=1.f*(y-lmy)/200;
		if (tw<0) tw=0;
		lmx=x; lmy=y;
		break;
		
	case DM_SCALE_ALL:
		y=win_h-y;
		ts+=1.f*(y-lmy)/200;
		if (ts<0) ts=0;
		lmx=x; lmy=y;
		break;
		
	case DM_SCALE_XY:
		y=win_h-y;
		sy+=1.f*(y-lmy)/200;
		sx+=1.f*(x-lmx)/200;
		if (ts<0) ts=0;
		lmx=x; lmy=y;
		break;
		
	case DM_ROTATE_W:
		y=win_h-y;
		tr+=1.f*(x-lmx)/200;
		tr+=sin(a2)*(y-lmy)/200;
		lmx=x; lmy=y;
		break;
	
	}
	glutPostRedisplay();
}

// callback para el click del mouse, guarda las coord para mover la "camara"
void mouse_cb (int button, int state, int x, int y){
	if (state==GLUT_DOWN && button==GLUT_LEFT_BUTTON) {
		if (glutGetModifiers()==GLUT_ACTIVE_SHIFT)
			cur_drag_mode=DM_CAMERA;
		else
			cur_drag_mode=def_drag_mode;
		lmx=x;
		lmy=win_h-y;
	}
}

// callback para el teclado
void keyboard_cb (unsigned char key,int x=0,int y=0) {
	switch (std::tolower(key)) {
	case 'h':
		show_help=!show_help;
		break;
	case 'b':
		show_bounding_box=!show_bounding_box;
		break;
	case 'a':
		show_axis_label=!show_axis_label;
		break;
	case 'l':
		show_full_lines=!show_full_lines;
		break;
	case 'w':
		show_w1_plano=!show_w1_plano;
		break;
	case 'i':
		show_w0_plano=!show_w0_plano;
		break;
	case '1':
		show_w1_ejes=!show_w1_ejes;
		break;
	case 'q':
		quad=!quad;
		break;
	case 'p':
		perspective=!perspective; 
		break;
	case 'e':
		show_ejes=!show_ejes; 
		break;
	case 'z': 
		tp=tr=tx=ty=0; tw=sx=sy=ts=1;
		break;
	case 'u': 
		def_drag_mode=DM_SCALE_ALL;
		break;
	case 'd': 
		def_drag_mode=DM_MOVE_XY;
		break;
	case 's': 
		def_drag_mode=DM_SCALE_W;
		break;
	case 'x': 
		def_drag_mode=DM_SCALE_XY;
		break;
	case 'r': 
		def_drag_mode=DM_ROTATE_W;
		break;
	case 't': 
		def_drag_mode=DM_ROTATE_X;
		break;
	case 'f':
		fullscreen=!fullscreen;
		if (fullscreen)	{
			win_x = glutGet((GLenum)GLUT_WINDOW_X);
			win_y = glutGet((GLenum)GLUT_WINDOW_Y); 
			lww=win_w; lwh=win_h;
			glutFullScreen();
		} else {
			glutReshapeWindow(lww,lwh);
			glutPositionWindow(win_x,win_y);
		}
		break;
	case 27:
		exit(0);
	}
	glutPostRedisplay();
}

// inicializaciones varias
void initialize() {
	glutInitDisplayMode (GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize (win_w, win_h);
	glutInitWindowPosition (win_x, win_y);
	glutCreateWindow ("P2 Demo");
	glutDisplayFunc (display_cb);
	glutReshapeFunc (reshape_cb);
	glutMotionFunc(motion_cb);
	glutMouseFunc(mouse_cb);
	glutKeyboardFunc(keyboard_cb);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

// funcion main
int main (int argc, char **argv) {
	glutInit (&argc, argv);
	initialize();
	mk_green_plane();
	glutMainLoop ();
	return 0;
}

