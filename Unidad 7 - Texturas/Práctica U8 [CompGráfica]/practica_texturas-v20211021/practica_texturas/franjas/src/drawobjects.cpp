#include <iostream> // cout
#include <cmath> // sin cos
#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glut.h>
#endif
using namespace std;

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

void drawBezierCurve(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_TEXTURE_1D);
  glPushMatrix();

  //Definicion de los puntos de control
  static const int order=4;//  order=degree+1
  static const GLfloat ctrlpoints[order][3]={
    {-4.0,-4.0,0.0},{-2.0,4.0,1.0},{2.0,-4.0,0.0},{4.0,4.0,1.0}
  };
  // color variable segun una curva de Bezier en RGBA
  static const GLfloat colors[order][4]={
    {0,0,1,1},{0,1,1,1},{1,1,0,1},{1,0,0,1}
  };

//  GL_MAP1_VERTEX_3 Three-dimensional vertices are produced
//  Low,High values of parameter u,
//  values to advance in the data between control points,
//  order=degree+1,
//  Pointer to data
  glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, order,&ctrlpoints[0][0]);
  glEnable(GL_MAP1_VERTEX_3);
  // Definicion de la curva parametrica de colores
  glMap1f(GL_MAP1_COLOR_4, 0.0, 1.0, 4, order,&colors[0][0]);
  glEnable(GL_MAP1_COLOR_4);

  int i;

  // 1/4 del original
  glScalef(.25,.25,.25);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
//  glColor4fv(line_color); // o esto o un mapa de colores

  // ------------esto--------------
  //poligonal de (lod) segmentos
  //por cada coordenada u
  /*glBegin(GL_LINE_STRIP);
    for (i=0;i<=lod;i++)
      glEvalCoord1f((GLfloat)i/lod);
  glEnd();*/
  // ------------o esto--------------
  //Dibuja la curva para todas las u juntas
  glMapGrid1f(lod,0,1);
  glEvalMesh1(GL_LINE,0,lod);
  // ------------------------------------

  // Dibuja el poligono de control
  glColor4fv(control_color);
  glBegin(GL_LINE_STRIP);
    for (i=0;i<order;i++)
      glVertex3fv(&ctrlpoints[i][0]);
  glEnd();

  // Dibuja los puntos de control
  glColor4fv(point_color);
  glBegin(GL_POINTS);
    for (i=0;i<order;i++)
      glVertex3fv(&ctrlpoints[i][0]);
  glEnd();

  glPopAttrib();
  glPopMatrix();
}

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
    glDisable(GL_TEXTURE_1D);
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
    glDisable(GL_TEXTURE_1D);
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
//      if (!blend) glEnable(GL_CULL_FACE);
      gluSphere(q,baseRadius,slices,stacks);
    }
    else if (tipo==disco)
      gluDisk(q,0,baseRadius,slices,1);
  }
  if (wire){
    // grilla de lineas
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_1D);
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

void drawColorCube(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glTranslated(-.5,-.5,-.5);
  glDisable(GL_LIGHTING);
//  if (!blend) glEnable(GL_CULL_FACE);

  // ejes
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_TEXTURE_1D);
  glBegin(GL_LINES);
    glColor3d(1,0,0); glVertex3d(1.1,0,0); glVertex3d(1.5,0,0);
    glColor3d(0,1,0); glVertex3d(0,1.1,0); glVertex3d(0,1.5,0);
    glColor3d(0,0,1); glVertex3d(0,0,1.1); glVertex3d(0,0,1.5);
  glEnd();
  glPopAttrib();

  if (relleno){ // caras
    glBegin(GL_QUADS);
      //b=0 normal hacia -b
      glColor4f(0,0,0,.5f); glVertex3i(0,0,0);
      glColor4f(0,1,0,.5f); glVertex3i(0,1,0);
      glColor4f(1,1,0,.5f); glVertex3i(1,1,0);
      glColor4f(1,0,0,.5f); glVertex3i(1,0,0);//color fijo

      //b=1 normal hacia +b
      glColor4f(0,0,1,.5f); glVertex3i(0,0,1);
      glColor4f(1,0,1,.5f); glVertex3i(1,0,1);
      glColor4f(1,1,1,.5f); glVertex3i(1,1,1);
      glColor4f(0,1,1,.5f); glVertex3i(0,1,1);//color fijo

      //g=0 normal hacia -g
      glColor4f(0,0,0,.5f); glVertex3i(0,0,0);
      glColor4f(1,0,0,.5f); glVertex3i(1,0,0);
      glColor4f(1,0,1,.5f); glVertex3i(1,0,1);
      glColor4f(0,0,1,.5f); glVertex3i(0,0,1);//color fijo

      //g=1 normal hacia +g
      glColor4f(0,1,0,.5f); glVertex3i(0,1,0);
      glColor4f(0,1,1,.5f); glVertex3i(0,1,1);
      glColor4f(1,1,1,.5f); glVertex3i(1,1,1);
      glColor4f(1,1,0,.5f); glVertex3i(1,1,0);//color fijo

      //r=0 normal hacia -r
      glColor4f(0,0,0,.5f); glVertex3i(0,0,0);
      glColor4f(0,0,1,.5f); glVertex3i(0,0,1);
      glColor4f(0,1,1,.5f); glVertex3i(0,1,1);
      glColor4f(0,1,0,.5f); glVertex3i(0,1,0);//color fijo

      //r=1 normal hacia +r
      glColor3f(1,0,0); glVertex3i(1,0,0);
      glColor3f(1,1,0); glVertex3i(1,1,0);
      glColor3f(1,1,1); glVertex3i(1,1,1);
      glColor3f(1,0,1); glVertex3i(1,0,1);//color fijo
    glEnd();
  }
  if (wire){ // aristas
    glDisable(GL_TEXTURE_1D);
    glBegin(GL_LINE_STRIP); // no hago loop por el color
      glColor3f(0,0,0); glVertex3i(0,0,0);
      glColor3f(0,1,0); glVertex3i(0,1,0);
      glColor3f(1,1,0); glVertex3i(1,1,0);
      glColor3f(1,0,0); glVertex3i(1,0,0);
      glColor3f(0,0,0); glVertex3i(0,0,0); // cierra en z=0

      glColor3f(0,0,1); glVertex3i(0,0,1); // arista vertical
      glColor3f(1,0,1); glVertex3i(1,0,1);
      glColor3f(1,1,1); glVertex3i(1,1,1);
      glColor3f(0,1,1); glVertex3i(0,1,1);
      glColor3f(0,0,1); glVertex3i(0,0,1); // cierra en z=1
    glEnd();

    // las tres aristas verticales faltantes
    glBegin(GL_LINES);
      glColor3f(0,1,0); glVertex3i(0,1,0);
      glColor3f(0,1,1); glVertex3i(0,1,1);
      glColor3f(1,1,0); glVertex3i(1,1,0);
      glColor3f(1,1,1); glVertex3i(1,1,1);
      glColor3f(1,0,0); glVertex3i(1,0,0);
      glColor3f(1,0,1); glVertex3i(1,0,1);
    glEnd();
  }

  glPopAttrib();
  glPopMatrix();
}

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
    glDisable(GL_TEXTURE_1D);
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
//    if (!blend) glEnable(GL_CULL_FACE);
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
    glDisable(GL_TEXTURE_1D);
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

void drawRectangulo(double x0=-.5, double y0=-.5, double x1=.5, double y1=.5){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glTranslated(x0,y0,0);
  glScaled(x1-x0,y1-y0,1);
  if (relleno){
    glBegin(GL_QUADS); // normal hacia el ojo
      glVertex3i(0,0,0);
      glVertex3i(1,0,0);
      glVertex3i(1,1,0);
      glVertex3i(0,1,0);
    glEnd();
  }
  if (wire){
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_1D);
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

void drawCubo(){ // cubo unitario centrado en 0,0
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

//  if (!blend) glEnable(GL_CULL_FACE);
  glDisable(GL_LIGHTING); // usa color y no material

  // esta dos veces el mismo codigo pero con dos lecturas distintas
/*
  // SE LEE DESDE EL FINAL HACIA ACA!!!!!!
  // esta es "la interpretacion oficial" OpenGL es una "maquina de estados"
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
  glColor3f(0.5f,1.0f,0.5f); drawRectangulo();  //  2 cuadrado verde claro adelante
  glTranslated(0,0,-1); glRotated(180,0,1,0);   //  3 me alejo y giro 180 alrededor de y
  glColor3f(0.0f,0.5f,0.0f); drawRectangulo();  //  4 cuadrado verde oscuro atras
  // ahora pongo las cuatro caras del costado
  // 5 voy a poner la cara de abajo
  //    bajo y avanzo hacia el centro de la cara de abajo
  //    giro 90 grados alrededor de x para quedar mirando hacia arriba
  glTranslated(0,-.5,-.5); glRotated(90,1,0,0);
  glColor3f(0.0f,0.0f,0.5f); drawRectangulo();  //  6 cuadrado azul oscuro abajo
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  //  7 me posiciono a la izquierda
  glColor3f(0.5f,0.0f,0.0f); drawRectangulo();  //  8 cuadrado rojo oscuro izquierda
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  //  9 me posiciono arriba
  glColor3f(0.5f,0.5f,1.0f); drawRectangulo();  // 10 cuadrado celeste arriba
  glTranslated(.5,0,-.5); glRotated(90,0,1,0);  // 11 me posiciono a la derecha
  glColor3f(1.0f,0.5f,0.5f); drawRectangulo();  // 12 cuadrado rosa derecha

  glPopAttrib();
  glPopMatrix();
}

void drawPlanoMovil(){
  static bool desde_aca=false; // para el segundo llamado
  if (wire&&!relleno&&!desde_aca){ // sin relleno ==> oculta lineas
    // dos pasadas 1) rellena, pero no dibuja 2) dibuja
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColorMask(false,false,false,true); // no dibuja en el framebuffer
    glDisable(GL_LIGHTING); // no calcula iluminacion
    glShadeModel(GL_FLAT);
    glDisable(GL_TEXTURE_1D);
    relleno=true; // para que arme el z-buffer
    desde_aca=true; // para que no entre de nuevo aca
    drawPlanoMovil(); // sin dibujar (solo actualiza el z-buffer)
    // ahora si dibuja
    glColorMask(true,true,true,true); // en el framebuffer
    relleno=false; // solo alambres (como venia pedido)
    glPopAttrib();
    drawPlanoMovil(); // dibuja
    desde_aca=false;
    return;
  }

  // dibuja el cubo y el plano movil
  // ejes y cubo fijos
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  gluLookAt(0,0,5,0,0,0,0,1,0); // misma distancia que en main
  glRotatef(15,1,0,0); glRotatef(-30,0,1,0);
  //--------
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glShadeModel(GL_FLAT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_1D);
  glColor3f(0.0f,0.0f,0.0f);
  glBegin(GL_LINES);
    glColor3f(1-0.5f,1-0.0f,1-0.0f); glVertex3f(-1.5, 0.0f, 0.0f);
    glColor3f(1-1.0f,1-0.5f,1-0.5f); glVertex3f( 1.5, 0.0f, 0.0f);
    glColor3f(1-0.0f,1-0.0f,1-0.5f); glVertex3f( 0.f,-1.5f, 0.0f);
    glColor3f(1-0.5f,1-0.5f,1-1.0f); glVertex3f( 0.f, 1.5f, 0.0f);
    glColor3f(1-0.5f,1-1.0f,1-0.5f); glVertex3f( 0.f, 0.0f, 1.0f);
    glColor3f(1-0.0f,1-0.5f,1-0.0f); glVertex3f( 0.f, 0.0f,-1.0f);
  glEnd();
  glScalef(.25,.25,.25);
  drawCubo();
  glPopMatrix();

  // normal y cuadrado movil
  glColor3f(0.0,0.0,0.0);
  glBegin(GL_LINES);
    glVertex3i(0,0,0);
    glVertex3i(0,0,1);
  glEnd();
  glPopAttrib();
  drawRectangulo(-.75,-.75,.75,.75);
}

void drawObjects(int change=0){
  static int which=0;
  static const int nobj=13; // objetos
  which=(which+change+nobj)%nobj;
  animado=false;
  switch (which){
    case 0:
      drawTeapot();
      if (cl_info&&change) cout << "Teapot" << endl;
      break;
    case 1:
      drawBezierSurface();
      if (cl_info&&change) cout << "BezierSurface" << endl;
      break;
    case 2:
      drawNurbsSurface();
      if (cl_info&&change) cout << "NurbsSurface" << endl;
      break;
    case 3:
      drawBezierCurve();
      if (cl_info&&change) cout << "BezierCurve" << endl;
      break;
    case 4:
      drawColorCube();
      if (cl_info&&change) cout << "ColorCube" << endl;
      break;
    case 5:
      drawDedo();
      if (cl_info&&change) cout << "Dedo" << endl;
      break;
    case 6:
      drawSphere();
      if (cl_info&&change) cout << "Esfera" << endl;
      break;
    case 7:
      drawDisk();
      if (cl_info&&change) cout << "Disco" << endl;
      break;
    case 8:
      drawCone();
      if (cl_info&&change) cout << "Cono" << endl;
      break;
    case 9:
      drawCylinder();
      if (cl_info&&change) cout << "Cilindro" << endl;
      break;
    case 10:
      drawRectangulo();
      if (cl_info&&change) cout << "Rectangulo" << endl;
      break;
    case 11:
      drawCubo();
      if (cl_info&&change) cout << "Cubo" << endl;
      break;
    case 12:
      drawPlanoMovil();
      if (cl_info&&change) cout << "Plano Movil" << endl;
      break;
  }
}

