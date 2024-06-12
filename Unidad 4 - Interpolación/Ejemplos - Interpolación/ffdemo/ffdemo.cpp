/**
* Este demo muestra un triangulo en dos vistas: 
* - en la vista 2d (inicialmente la principal), se pueden seleccionar y mover los 
*   vértices del triángulo con el mouse
* - en la vista 3d se ven los vectores que "miden" el area del triángulo y de cada
*   función de forma.
* Con el botón derecho del ratón se invierten las vistas
**/

#include <GL/glut.h>
#include <cmath>
#include "../common/OSD.hpp"
#include "../common/Algebra.hpp"
#include <ctime>
using namespace std;

Punto tri[3], pt, *sel=NULL, *vert_sel=NULL;
float alphas[4]={0,0,0,0}; 
int win_w=800,win_h=600, win_z, mouse_x,mouse_y;
GLfloat color_back_1[4]={1,1,1,1};
GLfloat color_back_2[4]={.9,.9,.9,1};
GLfloat color_grid[4]={0,0,0,.3};
GLfloat color_tri_in[4]={1,1,1,1};
GLfloat color_tri_pt[3][4]={ {0,.65,0,1}, {0,0,.65,1}, {.65,0,0,1} };
GLfloat color_tri_ln[4]={.5,.5,.5,1};
GLfloat color_ff_in[3][4]={ {1,0,0,.1}, {0,.6,0,.1}, {0,0,1,.1} };
GLfloat color_ff_no[4][4]={ {1,0,0, 1}, {0,.6,0, 1}, {0,0,1, 1}, {0,0,0,1} };
GLfloat color_ff_ln[4]={.5,.5,.5,1};
GLfloat color_pt[4]={.6,.6,0,1};

int viewport[2][4], main_viewport=0;

void glVertex2P(Punto p) { glVertex2f(p.x,p.y); }
void glVertex3P(Punto p) { glVertex3f(p.x,p.y,p.z); }

void reshape_cb (int w, int h) { 
	if (w==0||h==0) return;
	win_w=w; win_h=h; win_z=sqrt(win_h*win_h+win_w*win_w);
	
	int v1=main_viewport, v2=(main_viewport+1)%2;
		
	viewport[v1][0]=0;     viewport[v1][1]=0;
	viewport[v1][2]=win_w; viewport[v1][3]=win_h;
	
	viewport[v2][0]=2*win_w/3; viewport[v2][1]=0;
	viewport[v2][2]=win_w/3;   viewport[v2][3]=win_h/3;
	
	
}

void draw_stuff(bool draw_normals) {
	glLineWidth(1);
	glColor4fv(color_grid);
	glBegin(GL_LINES);
	if (draw_normals) {
		for(int i=-5;i<15;i++) { 
			glVertex2f(i*win_w/10+win_w/20,-win_h);
			glVertex2f(i*win_w/10+win_w/20,2*win_h);
			glVertex2f( -win_w,i*win_h/10+win_h/20);
			glVertex2f(2*win_w,i*win_h/10+win_h/20);
		}
	} else if (vert_sel) {
		int i=vert_sel-tri;
		Vector dir=tri[(i+2)%3]-tri[(i+1)%3]; 
		dir/=dir.Mod();
		if (dir.y>0) dir*=-1.f;
		
		Punto p1=tri[i], p2=tri[(i+1)%3];
		
		glVertex2P((p1-dir*win_z*2.f)); 
		glVertex2P((p1+dir*win_z*2.f));
		
		glVertex2P((p2-dir*win_z*2.f)); 
		glVertex2P((p2+dir*win_z*2.f));
	}
	glEnd();
	
	glPointSize(5);
	
	glColor4fv(color_tri_in);
	glBegin(GL_TRIANGLES);
	for(int i=0;i<3;i++) { 
		glVertex2P(tri[i]);
	}
	glEnd();
	
	glBegin(GL_TRIANGLES);
	for(int i=0;i<3;i++) { 
		if (sel && sel!=&pt && sel!=tri+(i+2)%3) continue;
		glColor4fv(color_ff_in[i]);
		glVertex2P(tri[i]);
		glVertex2P(pt);
		glVertex2P(tri[(i+1)%3]);
	}
	glEnd();
	
	glColor4fv(color_tri_ln);
	glLineWidth(3);
	glBegin(GL_LINES);
	for(int i=0;i<3;i++) { 
		glVertex2P(tri[i]);
		glVertex2P(tri[(i+1)%3]);
	}
	glEnd();
	
	glColor4fv(color_ff_ln);
	glLineWidth(1);
	glBegin(GL_LINES);
	for(int i=0;i<3;i++) { 
		glVertex2P(tri[i]);
		glVertex2P(pt);
		glVertex2P(pt);
		glVertex2P(tri[(i+1)%3]);
	}
	glEnd();
	
	glBegin(GL_POINTS);
	for(int i=0;i<3;i++) { 
		glColor4fv(color_tri_pt[i]);
		glVertex2P(tri[i]);
	}
	glEnd();
	
	glBegin(GL_POINTS);
	glColor4fv(color_pt);
	glVertex2P(pt);
	glEnd();
	
	if (!draw_normals) return;
	
	float un_tercio =1.f/3.f;
	Punto cent[4]; Vector norm[4];
	for(int i=0;i<3;i++) { 
		cent[i]=interp3(tri[i],tri[(i+1)%3],pt);
		norm[i]=((tri[(i+1)%3]-tri[i])%(pt-tri[i]));
	}
	cent[3]=interp3(tri[0],tri[1],tri[2]);
	norm[3]=(tri[1]-tri[0])%(tri[2]-tri[0]);
	double f=norm[3].Mod()/100;
	for(int i=0;i<4;i++) { norm[i]=norm[i]/f; alphas[i]=norm[i].z; }
	
//	glPointSize(5);
//	glBegin(GL_POINTS);
//	for(int i=0;i<4;i++) { 
//		glColor4fv(color_ff_no[i]);
//		glVertex3P(cent[i]);
//	}
//	glEnd();
	
	glLineWidth(3);
	glBegin(GL_LINES);
	for(int i=0;i<4;i++) { 
		glColor4fv(color_ff_no[i]);
		glVertex3P(cent[i]);
		glVertex3P((cent[i]+.7*norm[i]));
	}
	glEnd();
	const int aw = 6;
	for(int j=0;j<=aw;j++) { 
		glLineWidth(j+1);
		glBegin(GL_LINES);
		for(int i=0;i<4;i++) { 
			glColor4fv(color_ff_no[i]);
			glVertex3P(cent[i]+norm[i]*0.7);
			glVertex3P(cent[i]+norm[i]*(1.0-0.3*(j/double(aw))));
		}
		glEnd();
	}
	
}

void draw_2d() {
	
	glViewport(viewport[0][0],viewport[0][1],viewport[0][2],viewport[0][3]);
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,win_w,0,win_h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	
	glColor4fv(color_back_1);
	glBegin(GL_QUADS);
	glVertex2f(-10000,-10000);
	glVertex2f(-10000,+10000);
	glVertex2f(+10000,+10000);
	glVertex2f(+10000,-10000);
	glEnd();
	
	draw_stuff(false);
}

void draw_3d() {
	
	glViewport(viewport[1][0],viewport[1][1],viewport[1][2],viewport[1][3]);
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(-win_w/2,win_w/2,win_h/2,-win_h/2,0,win_z*2);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	
	glColor4fv(color_back_2);
	glBegin(GL_QUADS);
	glVertex2f(-1000,-1000);
	glVertex2f(-1000,+1000);
	glVertex2f(+1000,+1000);
	glVertex2f(+1000,-1000);
	glEnd();
	
	Punto c = interp3(tri[0],tri[1],tri[2]);
	gluLookAt( c.x-win_w/2,c.y-win_z/2,-win_z/2, c.x,c.y,0, 0,0,1);
	draw_stuff(true);
}

void display_cb() {
	glClear(GL_COLOR_BUFFER_BIT);
	if (main_viewport) {
		draw_3d();
		draw_2d();
	} else {
		draw_2d();
		draw_3d();
	}
	
	glLoadIdentity();
	glViewport(0,0,win_w,win_h);
	glColor3f(0,0,0);
	OSD<<"alpha_r = "<<alphas[0]/alphas[3]<<"\n";
	OSD<<"alpha_g = "<<alphas[1]/alphas[3]<<"\n";
	OSD<<"alpha_b = "<<alphas[2]/alphas[3]<<"\n";
	OSD.Render(win_w,win_h);
	
	glutSwapBuffers();
}


bool fix_mouse_coords(int &x, int &y) {
	x=((x-viewport[0][0])*win_w)/(viewport[0][2]);
	y=(((win_h-y)-viewport[0][1])*win_h)/(viewport[0][3]);
	return x>0 && x<win_w && y>0 && y<win_h;
}

void motion_cb(int x, int y) {
	fix_mouse_coords(x,y);
	if (sel) {
		sel->x=x;
		sel->y=y;
	} else {
		int dx=x-mouse_x, dy=y-mouse_y;
		for(int i=0;i<3;i++) { 
			tri[i].x+=dx;
			tri[i].y+=dy;
		}
		pt.x+=dx;
		pt.y+=dy;
		mouse_x=x; mouse_y=y;
	}
	glutPostRedisplay();
}

void mouse_cb(int button, int state, int x, int y) {
	fix_mouse_coords(x,y);
	if (button!=GLUT_LEFT) { 
		if (state==GLUT_DOWN) {
			main_viewport=(main_viewport+1)%2;
			reshape_cb(win_w,win_h);
			glutPostRedisplay();
		}
		return;
	}
	mouse_x=x; mouse_y=y;
	Punto paux(x,y,0);
	int mdist=(paux-pt).Mod2(); sel=&pt;
	for(int i=0;i<3;i++) { 
		int adist=(paux-tri[i]).Mod2();
		if (adist<mdist) { mdist=adist; sel=tri+i; }
	}
	if (mdist>(main_viewport?800:250)) sel=NULL;
	if (sel!=&pt) vert_sel=sel;
	glutPostRedisplay();
}

void initialize() {
	glutInitDisplayMode (GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize (win_w,win_h);
	glutInitWindowPosition (100,100);
	glutCreateWindow ("FFDemo");
	glutDisplayFunc (display_cb);
	glutReshapeFunc (reshape_cb);
	glutMouseFunc(mouse_cb);
	glutMotionFunc(motion_cb);
	
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(1,1,1,1);
	
	srand(time(0));
	for(int i=0;i<3;i++) { 
		tri[i]=Punto(rand()%win_w,rand()%win_h,0);
	}
	if ( ((tri[0]-tri[1])%(tri[2]-tri[1])).z<0) {
		std::swap(tri[0],tri[2]);
	}
	
	pt=interp3(tri[0],tri[1],tri[2]);
}

int main (int argc, char **argv) {
	glutInit (&argc, argv);
	initialize();
	glutMainLoop();
	return 0;
}

