#include <iostream> // cin, cout
#include <cstdlib> // exit
#include <cmath> // fabs sqrt
#include <cstring> // memcpy
#ifdef __APPLE__
# include <GLUT/glut.h>
#else
# include <GL/glut.h>
#endif

using namespace std;

///////////////////////////////////////////////////////////
// clase: puntos de cuatro floats
// MANEJESE CON CUIDADO!!!
  // ==> OJO: no admite peso 0 !!!!!!!!!!!!!!!!!
// 4 para poder hacer racionales y porque GLU pide 3D o 4D
// floats porque GLU pide floats
// tiene solo lo necesario para usarlos aca
// z siempre es 0
const size_t SZF=sizeof(float);
class p4f{
  float c[4];
public:
  p4f(double x=0, double y=0, double z=0, double w=1){
    c[0]=x; c[1]=y; c[2]=z; c[3]=w;
  }
  p4f(const p4f &p){memcpy(c,p.c,4*SZF);}

  operator const float*() const {return c;}

  p4f& operator=(const p4f &p){memcpy(c,p.c,4*SZF); return *this;}

  float& operator[](int i) {return c[i];}
  const float& operator[](int i) const {return c[i];}

  // modifica el peso pero mantiene el punto en el mismo lugar de R3
  // ==> OJO: no admite peso 0 !!!!!!!!!!!!!!!!!
  // (para hacer peso=0 poner p[3]=0 y ya esta)
  p4f& peso(float w){
    if (fabs(w)<1e-7) {if (w<0) w=-1e-7; else w=1e-7;}
    float f=w/c[3];
    c[0]*=f; c[1]*=f; c[2]*=f; c[3]=w; return *this;
  }

  bool cerca2D(const p4f &p, float r=1){
    float cw=c[3],pw=p[3];
    return (fabs(c[0]/cw-p[0]/pw)<=r && fabs(c[1]/cw-p[1]/pw)<=r);
  }

  p4f& neg(){c[0]=-c[0]; c[1]=-c[1]; c[2]=-c[2]; c[3]=-c[3]; return *this;}
  p4f operator -() const {return p4f(-c[0],-c[1],-c[2],-c[3]);}

  // asigna el punto interpolado
  p4f& lerp(const p4f& p0,const p4f& p1,float u){
    c[0]=(1-u)*p0[0]+u*p1[0];
    c[1]=(1-u)*p0[1]+u*p1[1];
    c[2]=(1-u)*p0[2]+u*p1[2];
    c[3]=(1-u)*p0[3]+u*p1[3];
    return *this;
  }
};
///////////////////////////////////////////////////////////

// variables
int
  w,h, // alto y ancho de la pantalla
  tol=10, // tolerancia para seleccionar un punto
  yclick, // y picado
  lod=32; // nivel de detalle (subdivisiones de lineas)

float // colores
  fondo[]={0.95f,0.98f,1.0f}, // color de fondo
  lc_c[]={.4f,.4f,.6f,1.f},   // poligono de control
  pc_c[]={.2f,.2f,.4f,1.f},   // puntos de control
  c_c[]={.8f,.4f,.4f,1.f},    // curva
  la_c[]={.6f,.8f,.6f,1.f},   // lineas accesorias
  p_c[]={.4f,0.f,0.f,1.f};    // punto de la curva

bool
  cl_info=true,     // informa por la linea de comandos;
  antialias=false;  // antialiasing

// DeCasteljau
int npc=0,MAXPC=0; p4f *pc=0, *pd=0;  // puntos de control y DeCalsteljau
int istep=-1; // pasos de interpolacion
int pcsel=-1; // punto seleccionado
float u=.5,u0,W0; // parametro variable y peso
bool negativo=false; // muestra o no pesos negativos

// funciones
void check_gl_error(){
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
    if (cl_info){
           if(errornum==GL_INVALID_ENUM)
             cout << "GL_INVALID_ENUM" << endl;
      else if(errornum==GL_INVALID_VALUE)
             cout << "GL_INVALID_VALUE" << endl;
      else if (errornum==GL_INVALID_OPERATION)
             cout << "GL_INVALID_OPERATION" << endl;
      else if (errornum==GL_STACK_OVERFLOW)
             cout << "GL_STACK_OVERFLOW" << endl;
      else if (errornum==GL_STACK_UNDERFLOW)
             cout << "GL_STACK_UNDERFLOW" << endl;
      else if (errornum==GL_OUT_OF_MEMORY)
             cout << "GL_OUT_OF_MEMORY" << endl;
    }
    errornum=glGetError();
  }
}

 void salir(){
  delete [] pd; delete [] pc;
  exit(EXIT_SUCCESS);
}

//--------
void dibuja(){
  if (!npc) return;
  glPointSize(3); glLineWidth(1);
  
  if (npc==1){ // dibuja el punto movil grueso
    glColor3fv(p_c);
    glBegin(GL_POINTS);
      glVertex4fv(pc[0]);
    glEnd();
    return;
  }

  int i,j;

  // puntos de control
  glColor4fv(pc_c); 
  glBegin(GL_POINTS);
  for (i=0;i<npc;i++) glVertex4fv(pc[i]);
  glEnd();
  // poligono de control
  glColor4fv(lc_c);
  glBegin(GL_LINE_STRIP);
  for (i=0;i<npc;i++) glVertex4fv(pc[i]);
  glEnd();

  // dibuja la curva
  glColor4fv(c_c); 
  // curva de Bezier de grado npc-1
  glMap1f(GL_MAP1_VERTEX_4, 0.0, 1.0, 4, npc, (const float*)pc);
  glEnable(GL_MAP1_VERTEX_4);
  glMapGrid1f(lod,0,1);
  glEvalMesh1(GL_LINE,0,lod);

  // dibuja las lineas auxiliares y el punto de la curva
  if (!istep) return;
  for (i=0;i<npc;i++) glVertex4fv(pd[i]=pc[i]);
  glColor3fv(la_c);
  
  for (j=0;j<istep-1;j++) {
    glBegin(GL_LINE_STRIP);
    for (i=0;i<npc-j-1;i++) glVertex4fv(pd[i].lerp(pd[i],pd[i+1],u));
    glEnd();
  }
  glColor3fv(p_c);glBegin(GL_POINTS);
    for (j=0;j<npc-istep;j++){
	    glVertex4fv(pd[j].lerp(pd[j],pd[j+1],u));
    }
  glEnd();

  if (negativo){
    for (i=1;i<npc-1;i++) pc[i].neg();
    negativo=false; dibuja(); negativo=true;
    for (i=1;i<npc-1;i++) pc[i].neg();
  }
}

//============================================================
// callbacks

//------------------------------------------------------------
// redibuja cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
  glClear(GL_COLOR_BUFFER_BIT);  // borra el buffer de pantalla
  dibuja();
  glutSwapBuffers();

  check_gl_error();
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb(int wi, int hi){
  w=wi;h=hi;
//  if (cl_info) cout << "reshape " << w << "x" << h << endl;

  glViewport(0,0,w,h); // region donde se dibuja

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(-w/2,w-w/2,h-h/2,-h/2,-1,1);

  glutPostRedisplay();
}

//------------------------------------------------------------
// Teclado y Mouse
// GLUT ACTIVE SHIFT Set if the Shift modifier or Caps Lock is active.
// GLUT ACTIVE CTRL  Set if the Ctrl modifier is active.
// GLUT ACTIVE ALT   Set if the Alt modifier is active.

short modifiers=0;  // ctrl, alt, shift (de GLUT)
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key==27){ // escape => exit
    get_modifiers();
    if (!modifiers) salir();
  }
  else if (key>='0' && key<='0'+npc-1){ 
	  istep=key-'0';
	  glutPostRedisplay();
  }
  else if (key=='a'||key=='A'){ // Antialiasing
    antialias=!antialias;
    if (antialias){
      glEnable(GL_BLEND);
      glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH);
      if (cl_info) cout << "Antialiasing" << endl;
    }
    else {
      glDisable(GL_BLEND);
      glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH);
      if (cl_info) cout << "Sin Antialiasing" << endl;
    }
  }
  else if (key=='+'){ // lod
    // no tiene sentido que divida mas que un segmento por pixel!!
    int maxd=0; double d; // maxima distancia entre puntos de control
    for (int i=0;i<npc-1;i++){
      d=fabs(pc[i+1][0]-pc[i][0]); if (d>maxd) maxd=d;
      d=fabs(pc[i+1][1]-pc[i][1]); if (d>maxd) maxd=d;
    }
    if (lod>=maxd) {lod=maxd; return;} // limita el maximo
    lod++;
    if (cl_info) {
      cout << "Nivel de Detalle: " << lod  << "               \r";
      cout.flush();
    }
  }
  else if (key=='-'){ // lod
    if (lod==2) return;
    lod--;
    if (cl_info) {
      cout << "Nivel de Detalle: " << lod  << "               \r";
      cout.flush();
    }
  }
  else if (key=='n'||key=='N'){ // negativo
    negativo=!negativo;
  }
  else if (key=='i'||key=='I'){ // info
    cl_info=!cl_info;
    cout << ((cl_info)? "Info" : "Sin Info") << endl;
    return;
  }
  else if (key==127){ // del => borra pt
    if (npc==0) return;
    for (int i=pcsel;i<npc-1;i++)
      pc[i]=pc[i+1];
    npc--; istep--; if (istep<0) istep=0;
    if (pcsel==npc) pcsel--;
    if (cl_info) {
      cout << "Pesos: {";
      for (int k=0;k<npc-1;k++) cout << pc[k][3] << ", ";
      cout << pc[npc-1][3] << "}                         \r" << flush;
    }
  }
  glutPostRedisplay();
}

// Special keys (non-ASCII)
//  GLUT KEY F[1,12] F[1,12] function key.
//  GLUT KEY LEFT Left directional key.
//  GLUT KEY UP Up directional key.
//  GLUT KEY RIGHT Right directional key.
//  GLUT KEY DOWN Down directional key.
//  GLUT KEY PAGE UP Page up directional key.
//  GLUT KEY PAGE DOWN Page down directional key.
//  GLUT KEY HOME Home directional key.
//  GLUT KEY END End directional key.
//  GLUT KEY INSERT Inset directional key.

// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT) salir();
  }
}

// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'a':
      Keyboard_cb('a');
      return;
    case '+':
      Keyboard_cb('+');
      return;
    case '-':
      Keyboard_cb('-');
      return;
    case 'n':
      Keyboard_cb('n');
      return;
    case 127: // del = borra
      Keyboard_cb(127);
      return;
    case 27: // esc = exit
      salir();
    }
}
//------------------------------------------------------------

// Movimientos del mouse
void Motion_cb(int x, int y){ // drag
  x-=w/2;y-=h/2;
  if (modifiers==GLUT_ACTIVE_CTRL){ // cambia el peso
//    float W=W0*pow(10.0,int(20.0*4.0*(yclick-y)/h)/20.0);
    float W=W0+5.0*(yclick-y)/h;
    if (pcsel>=0&&pcsel<npc) pc[pcsel].peso(W);
    if (cl_info) {
      cout << "Pesos: {";
      for (int k=0;k<npc-1;k++) cout << pc[k][3] << ", ";
      cout << pc[npc-1][3] << "}                         \r" << flush;
    }
  }
  else if (modifiers==GLUT_ACTIVE_SHIFT){ // calcula u
    u=u0+int(20.0*(yclick-y)/h)/20.0;
    if (cl_info) {cout << "\rparametro: " << u << "         " << flush;}
  }
  else{
    // mueve el punto
    // permite drag solo dentro del viewport
    // en caso contrario, sin zoom, no se pueden editar los que esten fuera
    if (x<-w/2) x=-w/2; else if (x>w-w/2-1) x=w-w/2-1;
    if (y<-h/2) y=-h/2; else if (y>h-h/2-1) y=h-h/2-1;
    p4f pm(x,y);
    // si se acerco a otro lo pega
    int i;
    for (i=0;i<npc;i++){
      if (i==pcsel) continue;
      if (!pm.cerca2D(pc[i],tol)) continue;
      pm=pc[i];
      break;
    }
    pm.peso(pc[pcsel][3]); pc[pcsel]=pm; // mueve
  }
  glutPostRedisplay();
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  x-=w/2;y-=h/2;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      p4f pm(x,y);
      int i;
      // verifica tolerancia
      for(i=0;i<npc;i++){
        if (!pm.cerca2D(pc[i],tol)) continue;
        pcsel=i;
        glutMotionFunc(Motion_cb);
        return;
      }
      // no pico cerca de ninguno
      get_modifiers();
      if (modifiers==GLUT_ACTIVE_CTRL    // cambia el peso
        ||modifiers==GLUT_ACTIVE_SHIFT){ // cambia el parametro
        yclick=y; u0=u; W0=pc[pcsel][3];
        glutMotionFunc(Motion_cb);
        return;
      }
      if (npc==MAXPC) return;
      // agrega entre medio
      for (i=npc;i>pcsel+1;i--) pc[i]=pc[i-1];
      pc[i]=pm;
      pcsel=i;
      npc++; istep++;
      if (cl_info) {
        cout << "Pesos: {";
        for (int k=0;k<npc-1;k++) cout << pc[k][3] << ", ";
        cout << pc[npc-1][3] << "}                         \r" << flush;
      }
      glutPostRedisplay();
      glutMotionFunc(Motion_cb);
    } // down
    else if (state==GLUT_UP){// fin del drag
      glutMotionFunc(NULL); // anula el callback para los drags
      modifiers=0;
    } // up
  } // left
}

//------------------------------------------------------------
int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<endl

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  // pide color RGB y double buffering
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);

  glutInitWindowSize(640,480); glutInitWindowPosition(250,100);

  glutCreateWindow("De Casteljau"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // resize
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // ctrl alt y shift
  glutMouseFunc(Mouse_cb); // clicks
  glutMotionFunc(Motion_cb); // clicks

  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("[Del] Borra Punto de Control", 127);
    glutAddMenuEntry("[+] +Nivel de Detalle     ", '+');
    glutAddMenuEntry("[-] -Nivel de Detalle     ", '-');
    glutAddMenuEntry("[n] -Pesos negativos      ", 'n');
    glutAddMenuEntry("[a] Antialiasing ON/OFF", 'a');
    glutAddMenuEntry("---------------------------", 255);
    glutAddMenuEntry("[i] Info ON/OFF", 'i');
    glutAddMenuEntry("[Esc] Exit", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo
  glDrawBuffer(GL_BACK);
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante
  glShadeModel(GL_FLAT); // no interpola color

  if (antialias){
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH);
  }
  else {
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH);
  }
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  

  // maximo orden y arrays de puntos
  glGetIntegerv(GL_MAX_EVAL_ORDER,&MAXPC);
  pc=new p4f[MAXPC]; pd=new p4f[MAXPC];

  // ========================
  // info
  if (cl_info){
      cout << "\n\n\nPuntos de control (Maximo " << MAXPC << "):\n"
         << "     Click izq -> Selecciona (tol. 10) o\n"
            "                    Agrega despues del seleccionado\n"
         << "      Drag izq -> Mueve el seleccionado\n"
         << "         <DEL> -> Borra el seleccionado\n"
         << " Ctrl+Drag izq -> Cambia el peso del seleccionado\n"
         << "Shift+Drag izq -> Cambia el parametro\n"
         << endl;
  }
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; //solo para que el builder se quede tranquilo (aqui nunca llega)
}
