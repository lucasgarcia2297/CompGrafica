#include <iostream> // OSD
#include <cmath> // sin cos
#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glut.h>
#endif
#include "OSD.hpp"

//==========================================
// globales

extern int lod,w,h,texmode;
extern bool wire,relleno,animado,smooth,cl_info,blend;
extern float line_color[],control_color[],point_color[],fondo[];

static const double PI=4*atan(1.0),R2G=180/PI;
//==========================================


//==========================================
// algunos objetos
//==========================================

void drawBezierSurface(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  static const GLfloat ctrlpoints[4][4][3]={
    {{-1.5,-1.5, 4.0},{-0.5,-1.5, 2.0},{0.5,-1.5,-1.0},{1.5,-1.5, 2.0}},
    {{-1.5,-0.5, 1.0},{-0.5,-0.5, 3.0},{0.5,-0.5, 0.0},{1.5,-0.5,-1.0}},
    {{-1.5, 0.5, 4.0},{-0.5, 0.5, 0.0},{0.5, 0.5, 3.0},{1.5, 0.5, 4.0}},
    {{-1.5, 1.5,-2.0},{-0.5, 1.5,-2.0},{0.5, 1.5, 0.0},{1.5, 1.5,-1.0}}
  };

  glMap2f(GL_MAP2_VERTEX_3,
    0.0, 1.0,  3, 4,
    0.0, 1.0, 12, 4,
    &ctrlpoints[0][0][0]);
  glEnable(GL_MAP2_VERTEX_3);

  // 1/2 del original
  glScalef(.5,.5,.5);

  // lod+1 puntos o lod segmentos en cada direccion
  glMapGrid2f(lod,0,1,lod,0,1);

  // si se dibuja 1ro el relleno no hay transparencia contra las lineas
  if (relleno){
    glFrontFace(GL_CW);// estan definidos al reves?
    glEnable(GL_AUTO_NORMAL); glEnable(GL_NORMALIZE); // si no lo renderiza mal
    glEnable(GL_LIGHTING);
    glEvalMesh2(GL_FILL,0,lod,0,lod);
  }

  if (wire){
    // grilla de lineas
    glDisable(GL_NORMALIZE); glDisable(GL_AUTO_NORMAL);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor4fv(line_color);
    glEvalMesh2(GL_LINE,0,lod,0,lod);

    // poliedro y puntos de control
    int i,j;
    for (i=0;i<3;i++) for (j=0;j<3;j++){
      glColor4fv(control_color);
      glBegin(GL_LINE_STRIP);
        glVertex3fv(&ctrlpoints[i+1][j  ][0]);
        glVertex3fv(&ctrlpoints[i  ][j  ][0]);
        glVertex3fv(&ctrlpoints[i  ][j+1][0]);
      glEnd();

      glColor4fv(point_color);
      glBegin(GL_POINTS);
        glVertex3fv(&ctrlpoints[i  ][j  ][0]);
        if (j==2)
          glVertex3fv(&ctrlpoints[i  ][j+1][0]);
        if (i==2){
          glVertex3fv(&ctrlpoints[i+1][j  ][0]);
          if (j==2)
            glVertex3fv(&ctrlpoints[i+1][j+1][0]);
        }
      glEnd();
    }
    glColor4fv(control_color);
    glBegin(GL_LINE_STRIP);
      for (j=0;j<4;j++)
        glVertex3fv(&ctrlpoints[3][j][0]);
      for (i=2;i>=0;i--)
        glVertex3fv(&ctrlpoints[i][3][0]);
    glEnd();
  }

  glPopAttrib();
  glPopMatrix();
}

void drawNurbsSurface(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  static GLfloat ctlpoints[4][4][3];
  static bool first=true;
  if (first){ // define los puntos de control (solo la primera vez que entra aqui)
    first=false;
    int u,v;
    for (u=0;u<4;u++) {
      for (v=0;v<4;v++) {
        ctlpoints[u][v][0] = 2.0f*(u-1.5f);
        ctlpoints[u][v][1] = 2.0f*(v-1.5f);

      if ((u==1||u==2)&&(v==1||v==2))
        ctlpoints[u][v][2] = 3.0;
      else
        ctlpoints[u][v][2] = -3.0;
      }
    }
  }

  static GLfloat knots[8] =
    {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};

  glScalef (0.3f, 0.3f, 0.3f);

  GLUnurbsObj *theNurb=gluNewNurbsRenderer();
  //GLU_SAMPLING_METHOD: (ojo: algunos dependen de la vista => incompatibles con display lists)
  //  GLU PATH LENGTH, (DEFAULT!!)
  //    GLU_SAMPLING_TOLERANCE es max length pixels edges of the tessellation polygons
  //  GLU OBJECT PATH LENGTH idem GLU PATH LENGTH object space
  //  GLU PARAMETRIC ERROR
  //    GLU PARAMETRIC TOLERANCE max dist pixels polygons/surfaces
  //  GLU OBJECT PARAMETRIC ERROR idem GLU PARAMETRIC ERROR object space
  //  GLU DOMAIN DISTANCE parametric sample points per unit length in u,v
  gluNurbsProperty(theNurb,GLU_SAMPLING_METHOD,GLU_DOMAIN_DISTANCE); // para usar lod
  gluNurbsProperty(theNurb,GLU_U_STEP,float(lod)); gluNurbsProperty(theNurb,GLU_V_STEP,float(lod));
//  gluNurbsProperty(theNurb,GLU_SAMPLING_METHOD,GLU_OBJECT_PATH_LENGTH); // no esta????
//  gluNurbsProperty(theNurb,GLU_SAMPLING_TOLERANCE,1.0/(lod+1));

  gluNurbsProperty(theNurb,GLU_CULLING,GL_TRUE); // si esta fuera del viewport
  if (relleno){
    gluNurbsProperty(theNurb,GLU_DISPLAY_MODE,GLU_FILL);

    glEnable(GL_LIGHTING);
    glEnable(GL_AUTO_NORMAL); glEnable(GL_NORMALIZE);

    gluBeginSurface(theNurb);
      gluNurbsSurface(theNurb,
          8, knots,
          8, knots,
          4 * 3,
          3,
          &ctlpoints[0][0][0],
          4, 4,
          GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
  }
  if (wire){
    // grilla de lineas
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE); glDisable(GL_AUTO_NORMAL);
    glDisable(GL_TEXTURE_2D);
    glColor4fv(line_color);
    gluNurbsProperty(theNurb,GLU_DISPLAY_MODE,GLU_OUTLINE_POLYGON);
    gluBeginSurface(theNurb);
      gluNurbsSurface(theNurb,
          8, knots,
          8, knots,
          4 * 3,
          3,
          &ctlpoints[0][0][0],
          4, 4,
          GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
  }
  gluDeleteNurbsRenderer(theNurb);
  glPopAttrib();
  glPopMatrix();
}

enum cuadrica{cono,esfera,cilindro,disco};
static void drawQuadric(cuadrica tipo){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  GLdouble baseRadius=1, topRadius=.2, height=1.5;
  GLint slices=lod, stacks=lod;

  // preposiciona
  if (tipo!=disco) glRotated(90,-1,0,0);
  if (tipo==cono||tipo==cilindro) glTranslated(0,0,-height/2);

  GLUquadricObj *q=gluNewQuadric();

  if (relleno){
    gluQuadricNormals(q,(smooth)? GLU_SMOOTH : GLU_FLAT);
    gluQuadricOrientation(q,GLU_OUTSIDE);
    //GLU FILL, GLU LINE, GLU POINT or GLU SILHOUETTE
    gluQuadricDrawStyle(q,GLU_FILL);
    if (tipo==cono)
      gluCylinder(q,baseRadius,topRadius,height,slices,stacks);
    else if (tipo==cilindro)
      gluCylinder(q,baseRadius,baseRadius,height,slices,stacks);
    else if (tipo==esfera){
      if (!blend) glEnable(GL_CULL_FACE);
      gluSphere(q,baseRadius,slices,stacks);
    }
    else if (tipo==disco)
      gluDisk(q,0,baseRadius,slices,1);
  }
  if (wire){
    // grilla de lineas
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor4fv(line_color);
    gluQuadricNormals(q,GLU_NONE);
//    gluQuadricDrawStyle(q,GLU_SILHOUETTE);
    gluQuadricDrawStyle(q,GLU_LINE);
    if (tipo==cono)
      gluCylinder(q,baseRadius,topRadius,height,slices,stacks);
    else if (tipo==cilindro)
      gluCylinder(q,baseRadius,baseRadius,height,slices,stacks);
    else if (tipo==esfera)
      gluSphere(q,baseRadius,slices,stacks);
    else if (tipo==disco)
      gluDisk(q,0,baseRadius,slices,1);
  }
  gluDeleteQuadric(q);
  glPopAttrib();
  glPopMatrix();
}
void drawCone(){drawQuadric(cono);}
void drawCylinder(){drawQuadric(cilindro);}
void drawSphere(){drawQuadric(esfera);}
void drawDisk(){drawQuadric(disco);}

void drawTeapot(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  // tetera
  glFrontFace(GL_CW);
  if (relleno){
    glEnable(GL_LIGHTING);
    glutSolidTeapot(1);
  }
  if (wire){
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor4fv(line_color);
    glutWireTeapot(1);
  }
  glPopAttrib();
}

//dedo de largo 1 apuntando hacia z
//angulos hacia abajo (eje x)
void draw1Dedo(GLUquadricObj* q,double a[3]){
  glPushMatrix();
  static const double f=1/(3+2+1.5);
  glScaled(f,f,f);
  glRotated(a[0],1,0,0);
  gluSphere(q,1,lod,lod);
  gluCylinder(q,1,1,3,lod,lod);
  glTranslated(0,0,3);
  glRotated(a[1],1,0,0);
  gluSphere(q,1,lod,lod);
  gluCylinder(q,1,1,2,lod,lod);
  glTranslated(0,0,2);
  glRotated(a[2],1,0,0);
  gluSphere(q,1,lod,lod);
  gluCylinder(q,1,1,1.5,lod,lod);
  glTranslated(0,0,1.5);
  gluSphere(q,1,lod,lod);
  glPopMatrix();
}

void drawDedo(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  GLUquadricObj *q=gluNewQuadric();
  glScalef(1.5,1.5,1.5);
  glRotated(90,0,1,0);// para que se vea hacia x

  static double t=0;
  double s1=sin(t),s2=sin(t-PI/2);
  double a1[3]={-45+30*s1,15+5*s1,22.5+22.5*s1},
         a2[3]={-45+30.5*s2,15+5*s2,22.5+22.5*s2};
  t+=10/R2G; // de a 10 grados

  if (relleno){
    if (!blend) glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE); glEnable(GL_AUTO_NORMAL);
    gluQuadricNormals(q,(smooth)? GLU_SMOOTH : GLU_FLAT);
    gluQuadricOrientation(q,GLU_OUTSIDE);
    gluQuadricDrawStyle(q,GLU_FILL);
    draw1Dedo(q,a1);
    glPushMatrix();
    glTranslated(.35,0,0);
    glRotated(5,0,1,0);
    draw1Dedo(q,a2);
    glPopMatrix();
  }
  if (wire){ // grilla de lineas
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE); glDisable(GL_AUTO_NORMAL);
    glDisable(GL_TEXTURE_2D);
    gluQuadricNormals(q,GLU_NONE);
    gluQuadricDrawStyle(q,GLU_LINE);
    glColor4fv(line_color);
    draw1Dedo(q,a1);
    glPushMatrix();
    glTranslated(.35,0,0);
    glRotated(5,0,1,0);
    draw1Dedo(q,a2);
    glPopMatrix();
  }
  gluDeleteQuadric(q);
  glPopAttrib();
  glPopMatrix();
  animado=true;
}

void drawRectangulo_impl(double x0, double y0, double x1, double y1){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glTranslated(x0,y0,0);
  glScaled(x1-x0,y1-y0,1);
  if (relleno){
    glBegin(GL_QUADS); // normal hacia el ojo
      glTexCoord2f(-.2,1);
      glVertex2i(0,0);
      glTexCoord2f(1.2,1);
      glVertex2i(1,0);
      glTexCoord2f(1.2,0);
      glVertex2i(1,1);
      glTexCoord2f(-.2,0);
      glVertex2i(0,1);
    glEnd();
  }
  if (wire){
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor4fv(line_color);
    glBegin(GL_LINE_LOOP);
      glVertex3i(0,0,0);
      glVertex3i(1,0,0);
      glVertex3i(1,1,0);
      glVertex3i(0,1,0);
    glEnd();
  }

  glPopAttrib();
  glPopMatrix();
}
  
void drawRectangulo(){
  drawRectangulo_impl(-2,-2,1.5,1.5);
}

void drawCubo(){ // cubo unitario centrado en 0,0
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  if (!blend) glEnable(GL_CULL_FACE);
  glDisable(GL_LIGHTING); // usa color y no material

  // esta dos veces el mismo codigo pero con dos lecturas distintas
/*
  // SE LEE DESDE EL FINAL HACIA ACA!!!!!!
  // esta es "la interretacion oficial" OpenGL es una "maquina de estados"
  // define un estado -> actua en ese estado -> define otro -> actua
  // cada estado definido se aplica a lo que sigue
  // Maquina ensambladora en 0,0: El modelo se mueve
  glTranslated(0,0,.5);                         // 11 adelante para centrar
  glColor3f(0.5f,1.0f,0.5f); drawRectangulo();  // 10 cuadrado verde claro
  // 9 gira 180 alrededor de y y aleja
  glTranslated(0,0,-1); glRotated(180,0,1,0);
  glColor3f(0.0f,0.5f,0.0f); drawRectangulo();  //  9 cuadrado verde oscuro
  glTranslated(0,-.5,-.5); glRotated(90,1,0,0);
  // 8 gira 90 alrededor de x, baja y aleja
  // por ultimo gerena las dos tapas
  glColor3f(0.0f,0.0f,0.5f); drawRectangulo();  //  7 cuadrado azul oscuro
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  //  6 posiciona
  glColor3f(0.5f,0.0f,0.0f); drawRectangulo();  //  5 cuadrado rojo oscuro
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  //  4 posiciona
  glColor3f(0.5f,0.5f,1.0f); drawRectangulo();  //  3 cuadrado celeste
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);
  // 2 primero gira 90 alrededor de y despues mueve hacia la derecha y atras
  //    (para conservar normal hacia afuera: 90 y derecha o -90 e izquierda)
  glColor3f(1.0f,0.5f,0.5f); drawRectangulo();  // 1 cuadrado rosa
  // primero genera las cuatro caras del costado
*/

  // Esta interpretacion es distinta pero asi se "puede leer"
  // Operario ensamblador: el operario se mueve con "su" sistema de coordenadas
  // SE LEE NORMALMENTE
  glTranslated(0,0,.5);                         //  1 me muevo adelante para centrar
  // primero pongo las caras delantera y trasera
  glColor3f(0.5f,1.0f,0.5f); drawRectangulo_impl(-0.5,-0.5,+0.5,+0.5);  //  2 cuadrado verde claro adelante
  glTranslated(0,0,-1); glRotated(180,0,1,0);   //  3 me alejo y giro 180 alrededor de y
  glColor3f(0.0f,0.5f,0.0f); drawRectangulo_impl(-0.5,-0.5,+0.5,+0.5);  //  4 cuadrado verde oscuro atras
  // ahora pongo las cuatro caras del costado
  // 5 voy a poner la cara de abajo
  //    bajo y avanzo hacia el centro de la cara de abajo
  //    giro 90 grados alrededor de x para quedar mirando hacia arriba
  glTranslated(0,-.5,-.5); glRotated(90,1,0,0);
  glColor3f(0.0f,0.0f,0.5f); drawRectangulo_impl(-0.5,-0.5,+0.5,+0.5);  //  6 cuadrado azul oscuro abajo
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  //  7 me posiciono a la izquierda
  glColor3f(0.5f,0.0f,0.0f); drawRectangulo_impl(-0.5,-0.5,+0.5,+0.5);  //  8 cuadrado rojo oscuro izquierda
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  //  9 me posiciono arriba
  glColor3f(0.5f,0.5f,1.0f); drawRectangulo_impl(-0.5,-0.5,+0.5,+0.5);  // 10 cuadrado celeste arriba
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  // 11 me posiciono a la derecha
  glColor3f(1.0f,0.5f,0.5f); drawRectangulo_impl(-0.5,-0.5,+0.5,+0.5);  // 12 cuadrado rosa derecha

  glPopAttrib();
  glPopMatrix();
}

void drawObjects(int change=0){
  static int which=0;
  animado=false;
  
  struct P { void(*func)(); const char *name; };
  P v[] = { 
    P{drawTeapot,"Tetera"},
    P{drawBezierSurface,"Superfice Bezier"},
    P{drawNurbsSurface,"Superficie NURBS"},
    P{drawDedo,"Dedo"},
    P{drawSphere,"Esfera"},
    P{drawDisk,"Dico"},
    P{drawCone,"Cono"},
    P{drawCylinder,"Cilindro"},
    P{drawRectangulo,"Rectangulo"},
    P{drawCubo,"Cubo"},
  };
  int nobj = sizeof(v)/sizeof(v[0]);
  which=(which+change+nobj)%nobj;
  v[which].func();
  if (change) OSD << v[which].name << "\n";
}

