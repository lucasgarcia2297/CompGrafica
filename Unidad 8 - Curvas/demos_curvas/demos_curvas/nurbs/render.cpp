#ifdef __APPLE__
# include <OpenGL/gl.win_h>
# include <OpenGL/glu.win_h>
# include <GLUT/glut.win_h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif
#include <cmath>
#include "render.h"
#include "nurbs.h"
#include "OSD.hpp"

constexpr int SCALE = 2;
constexpr int LW_1 = 1*SCALE;
constexpr int LW_2 = 2*SCALE;
constexpr int PS_1 = 4*SCALE;
constexpr int PS_2 = 7*SCALE;
constexpr int PS_3 = 10*SCALE;

//extern const int MAX_DETAIL=499;
extern const int MARGIN;

static float // colores
	color_fondo[]={1,1,1},       // color de fondo
	color_cline[]={.2f,.2f,.4f}, // poligono de control
	color_cpoint[]={.3f,0.3f,0.3f}, // puntos de control
	color_nurbs[]={.8f,.6f,.6f}, // curva
	color_knot[]={.4f,0.f,0.f}, // line de knots
	color_detail[]={.6f,.8f,.6f}, // lineas accesorias
	color_new[]={.8f,.6f,.6f}, // nuevo nodo
	color_texto[]={.3f,0.2f,0.8f}; // osd

extern int knots_nodes[ARRAY_MAX];

extern NURBS nurbs;

extern int win_w,win_h;

extern float sel_x,sel_y;
extern int sel_control, sel_knot;
extern bool sel_detail;
extern int detail_pos;
extern int offset_x, offset_y;
extern float last_w, last_ox, last_oy;
extern int knot_line;
extern int decasteljau_level;

extern bool draw_basis;
extern bool draw_knots;
extern bool draw_polygon;
extern bool draw_control;
extern bool draw_detail;
extern bool draw_kline;
extern bool draw_w1;
extern bool draw_parts;
extern bool sel_on_curve;


static void glColorHSV(float h, float s, float v) {
	
	if(h >= 360.f) h = 0.f; else h /= 60.f;
	
	int i = (int)h;
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - (s * f));
	float t = v * (1.f - (s * (1.f - f)));
	
	switch(i) {
	case 0:  glColor3f(v,t,p); return;
	case 1:  glColor3f(q,v,p); return;
	case 2:  glColor3f(p,v,t); return;
	case 3:  glColor3f(p,q,v); return;
	case 4:  glColor3f(t,p,v); return;
	default: glColor3f(v,p,q); return;
	}
	
}

static void glColorISV(int i, float s, float v) {
	constexpr float golden_ration = 1.61803398875;
	float hue = std::fmod(i*360.f/golden_ration,360.f);
	glColorHSV(hue,s,v);
}

static void RenderPolygon(NURBS &nurbs) {
	glLineWidth(LW_1);
	glColor3fv(color_cline);
	glBegin(GL_LINE_STRIP);
	for (int i=0;i<nurbs.num;i++)
		glVertex4fv(nurbs.controls[i]);
	glEnd();
}

static void RenderW1(NURBS &nurbs) {
	if (!nurbs.WellDefined()) return;
	int i;
	for (i=0;i<nurbs.num;i++){
		float win_w=nurbs.controls[i][W];
		nurbs.controls[i][X]/=win_w;
		nurbs.controls[i][Y]/=win_w;
	}
	glLineWidth(LW_1);
	glColor3fv(color_nurbs);
	static GLUnurbs *renderer = gluNewNurbsRenderer();
	gluNurbsProperty(renderer, GLU_SAMPLING_TOLERANCE, nurbs.detail);
	gluBeginCurve(renderer);
	gluNurbsCurve(renderer, nurbs.knum, nurbs.knots, 4, (float*)nurbs.controls, nurbs.order, GL_MAP1_VERTEX_3);
	gluEndCurve(renderer);
	for (i=0;i<nurbs.num;i++){
		float win_w=nurbs.controls[i][W];
		nurbs.controls[i][X]*=win_w;
		nurbs.controls[i][Y]*=win_w;
	}
//	gluDeleteNurbsRenderer(renderer); // no porque es static
}

static void RenderCurve(NURBS &nurbs, bool color=true) {
	if (!nurbs.WellDefined()) return;
	// dibujar la curva real
	glLineWidth(LW_2);
	if (color) glColor3fv(color_nurbs);
	static GLUnurbs *renderer = gluNewNurbsRenderer();
	gluNurbsProperty(renderer, GLU_SAMPLING_TOLERANCE, nurbs.detail);
	gluBeginCurve(renderer);
	gluNurbsCurve(renderer, nurbs.knum, nurbs.knots, 4, (float*)nurbs.controls, nurbs.order, GL_MAP1_VERTEX_4);
	gluEndCurve(renderer);
//	gluDeleteNurbsRenderer(renderer); // no porque es static
}
static void RenderCurveParts(NURBS &nurbs) {
	for(int i=nurbs.order-1,k=1;i<nurbs.num;i++,++k) { 
		glColorISV(k,1,.9);
		NURBS n2(nurbs);
		int k1 = n2.InsertKnot(nurbs.knots[i],true);
		for(int j=0;j<k1-n2.order+1;j++) n2.DeleteControl(0);
		RenderCurve(n2,false);
	}
}

static void RenderKnots(NURBS &nurbs) {
	// dibujar los knot sobre la curva
	nurbs.CalcKNots();
	glColor3fv(color_knot);
	glPointSize(PS_1);
	glBegin(GL_POINTS);
	for (int i=nurbs.order;i<nurbs.knum-nurbs.order;i++) {
		glVertex2iv(nurbs.knots_points[i]);
	}
	glEnd();
}

static void RenderPoints(NURBS &nurbs, bool use_colors) {
	// draw control points
	glColor3fv(color_cpoint);
	glPointSize(PS_1);
	glBegin(GL_POINTS);
	for (int i=0;i<nurbs.num;i++){
		if (use_colors) glColorISV(i,1,1);
		float win_w=nurbs.controls[i][W];
		if (win_w>0) glVertex4fv(nurbs.controls[i]);
		else glVertex4f(-nurbs.controls[i][X],-nurbs.controls[i][Y],0,-nurbs.controls[i][W]);
	}
	glEnd();
}

static float BSplineBasisFunc(float knots[], int i, int k, float t) {
	if (t<knots[i] || t>knots[i+k]) return 0;
	if (k==1)
		return (t>=knots[i] && t<knots[i+1])?1:0;
	else return 
		(t-knots[i])/((knots[i+k-1]-knots[i]==0?1:knots[i+k-1]-knots[i]))*BSplineBasisFunc(knots,i,k-1,t)
		+(knots[i+k]-t)/((knots[i+k]-knots[i+1]==0?1:knots[i+k]-knots[i+1]))*BSplineBasisFunc(knots,i+1,k-1,t);
};

static void RenderBasisFunctions() {
	
	auto func_draw_basis = [](int i, float t0, float t1) {
		glBegin(GL_LINE_STRIP);
		float knots_line_len = win_w-MARGIN-MARGIN;
		int x0 = MARGIN+t0*knots_line_len;
		int x1 = MARGIN+t1*knots_line_len;
		float dt = (t1-t0)/(x1-x0), t=t0;
		for (int x=x0;x<x1;++x,t+=dt) {
			float y = BSplineBasisFunc(nurbs.knots,i,nurbs.order,t);
			glVertex2f(x,win_h-MARGIN*2-y*win_h/3);
		}
		glEnd();			
	};
	
	glColor3fv(color_knot);
	for (int i=0;i<nurbs.num;i++) { // por cada pt de control
		glLineWidth(sel_control==i?LW_2:LW_1);
		if (!draw_parts) glColorISV(i,.5,1);
		func_draw_basis(i,0,1);
	}
	
	if (draw_parts) {
		glLineWidth(LW_2);
		for(int k=0;k<nurbs.num-nurbs.order+1;++k) {
			float t0 = nurbs.knots[k+nurbs.order-1];
			float t1 = nurbs.knots[k+nurbs.order];
			glColorISV(k+1,.7,1);			
			for (int i=0;i<nurbs.num;i++) { // por cada pt de control
				func_draw_basis(i,t0,t1);
			}
		}
	}
	
	if (sel_knot!=SEL_NONE) {
		glColor3fv(color_knot);
		glBegin(GL_LINES);
//		sel_knot_u = nurbs.knots[sel_knot];
//		sel_knot_u = float(sel_x-MARGIN)/knot_line;
		glVertex2f(sel_x,win_h-MARGIN*2);
		glVertex2f(sel_x,win_h-MARGIN*2-win_h/3);
		glEnd();
	}
	
}
	


// callback del display de la ventana de imagenes
void display_cb() {
	
	const auto FONT = GLUT_BITMAP_HELVETICA_12;
	
	glClearColor(color_fondo[0],color_fondo[1],color_fondo[2],1);
	
	glClear(GL_COLOR_BUFFER_BIT);

	if (offset_x!=0 || offset_y!=0) { // desplazar si esta pendiente un panning
		nurbs.Move(offset_x,offset_y);
		last_ox-=offset_x;
		last_oy-=offset_y;
		offset_x=0;
		offset_y=0;
	}
	
	
	// determinar el u si hay un pto seleccionado
	float sel_knot_u = -1;
	if (sel_knot==SEL_NEW) sel_knot_u = float(sel_x-MARGIN)/knot_line;
	else if (sel_knot!=SEL_NONE) 
		sel_knot_u = nurbs.knots[sel_knot];
	if (sel_knot_u<0||sel_knot_u>1) sel_knot_u = -1;

	if (draw_polygon) RenderPolygon(nurbs);
	if (nurbs.WellDefined()) {
		if (draw_w1) RenderW1(nurbs);
		if (draw_parts) RenderCurveParts(nurbs); 
		else RenderCurve(nurbs);
	}
	if (draw_knots)   RenderKnots(nurbs);
	if (draw_control) RenderPoints(nurbs,!draw_parts && sel_knot_u<0);

	// marcar el pto de control seleccionado
	if (sel_control!=SEL_NONE) {
		glColorISV(sel_control,1,1);
		glPointSize(PS_2);
		float* p=nurbs.controls[sel_control];
		glBegin(GL_POINTS);
		if (p[W]>0) glVertex4fv(p);
		else glVertex4f(-p[X],-p[Y],0,-p[W]);
		glEnd();
	}

	if (draw_detail) {
		// draw detail line
		glLineWidth(LW_1);
		glColor3fv(color_detail);
		glBegin(GL_LINES);
		glVertex2i(win_w-MARGIN, MARGIN);
		glVertex2i(win_w-MARGIN,win_h-MARGIN-MARGIN);
		glEnd();
		
		// draw detail point
		glPointSize(sel_detail?PS_2:PS_1);
		glBegin(GL_POINTS);
		glVertex2f(win_w-MARGIN,detail_pos);
		glEnd();
	}

	if (draw_basis) RenderBasisFunctions();
	
	int kh=win_h-MARGIN;
	if (draw_kline) {
		// draw knots line
		glLineWidth(LW_1);
		glColor3fv(color_knot);
		glBegin(GL_LINES);
		glVertex2i(MARGIN,win_h-MARGIN);
		glVertex2i(win_w-MARGIN,win_h-MARGIN);
		glEnd();
		
		// draw knots
		glPointSize(PS_1);
		glBegin(GL_POINTS);
		float lk=-1;
		int count=0;
		for (int i=1;i<nurbs.knum-1;i++) {
			if (lk==nurbs.knots[i]) {
				knots_nodes[i]=knots_nodes[i-1];
				count++;
			} else {
				if (count) {
					glEnd();
					glRasterPos2f(MARGIN+knot_line*lk-5,win_h-MARGIN-5);
					glutBitmapCharacter(FONT, 'x');
					glutBitmapCharacter(FONT, count+'1');
					glBegin(GL_POINTS);
					count=0;
				}
				glVertex2i(knots_nodes[i] = int(MARGIN+knot_line*(lk=nurbs.knots[i])),kh);
			}
		}
		glEnd();
		if (count) {
			glRasterPos2f(MARGIN+knot_line*lk-10,win_h-MARGIN-5);
			glutBitmapCharacter(FONT, 'x');
			glutBitmapCharacter(FONT, count+'1');
		}
	}
	if (sel_knot_u>=0) { // dibujar el knot seleccionado
		if (sel_knot==SEL_NEW) glColor3fv(color_new);
		glPointSize(PS_2);
		glBegin(GL_POINTS);
		// en la linea de knots
		if (draw_kline) glVertex2i(MARGIN+sel_knot_u*(win_w-2*MARGIN),kh);
		// en la curva
		bool sel_knot_u_is_ok = sel_knot_u>=nurbs.knots[nurbs.order-1] && sel_knot_u<=nurbs.knots[nurbs.num];
		int rep_knots;
		if (sel_knot_u_is_ok) glVertex4fv(nurbs.FindPoint(sel_knot_u,&rep_knots));
		glEnd();
		
		// segmentos de decasteljau/bloosoming
		if (decasteljau_level && sel_knot_u_is_ok) {
			decasteljau_level = decasteljau_level%(nurbs.order-1);
			glLineWidth(LW_1); glBegin(GL_LINES);
			for(int i=1;i<=decasteljau_level;i++) { 
				glColorISV(i+1,1,.75);
				for(int j=i;j<nurbs.order-1-rep_knots;j++) { 
					glVertex2fv(nurbs.aux_c[i][j]);
					glVertex2fv(nurbs.aux_c[i][j+1]);
				}
			}
			glEnd();
			glPointSize(PS_1); glBegin(GL_POINTS);
			for(int i=1;i<=decasteljau_level;i++) { 
				glColorISV(i+1,1,.75);
				for(int j=i;j<nurbs.order-rep_knots;j++) { 
					glVertex2fv(nurbs.aux_c[i][j]);
				}
			}
			glEnd();
		}
	}
	
	glColor3fv(color_texto);
	OSD.Render(win_w,win_h);
	
	glutSwapBuffers();
}
