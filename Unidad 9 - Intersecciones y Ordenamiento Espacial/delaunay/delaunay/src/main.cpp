#include <iostream> // cin, cout
#include <cstdlib> // exit
#include <ctime> // time
#include <cmath> // fabs
#include <GL/glut.h>

#include "utiles.h"
#include "p2e.h"
#include "delaunay.h"

using namespace std;

// comentar este bloque si se quiere una ventana de comando
//#ifdef _WIN32
// #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
//#endif

// variables
static int
  tol=15; // tolerancia para seleccionar un punto

static GLfloat // luces y colores en float;
  linewidth=1,pointsize=5,
  fondo[]={0.95f,0.98f,1.0},       // color de fondo
  delaunay_color[]={.2f,.2f,.6f},
  voronoi_color[]={.6f,.2f,.2f},
  circle_color[]={.2f,.6f,.6f},
  vertex_color[]={.1f,.1f,.3f},
  center_color[]={.3f,.1f,.1f};

static bool
  changemenu=false, // avisa que hay que modificar el menu
  cl_info=true;     // informa por la linea de comandos;

static short modifiers=0;  // ctrl, alt, shift (de GLUT)

// delaunay
static delaunay D; // una instancia de la clase
static pila_ptr<p2e> puntos; // listas de puntos
static p2e *pt=0; // el punto que se agrega mueve o borra
static bool dibujar_c=false, dibujar_t=false, dibujar_v=false;

// funciones
static void check_gl_error(){
//#ifdef _DEBUG
  // chequea errores
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
//#endif // _DEBUG
}

static void salir(){
  exit(EXIT_SUCCESS);
}

static void circulo_Bresenham(int xc, int yc, int r) {
  glPointSize(linewidth); // si, la circunferencia es una linea
  glBegin(GL_POINTS);
  int x=0, y=r, h=1-r, E=3, SE=5-(r<<1);
  while (y>x){
    glVertex2i( x+xc, y+yc);
    glVertex2i( x+xc,-y+yc);
    glVertex2i(-x+xc, y+yc);
    glVertex2i(-x+xc,-y+yc);

    glVertex2i( y+xc, x+yc);
    glVertex2i( y+xc,-x+yc);
    glVertex2i(-y+xc, x+yc);
    glVertex2i(-y+xc,-x+yc);
    if (h>=0){ // SE
      h+=SE; E+=2; SE+=4;
      y--;
    }
    else { // E
      h+=E; E+=2; SE+=2;
    }
    x++;
  }
  // x==y => no hace falta intercambiar
  glVertex2i( x+xc, y+yc);
  glVertex2i( x+xc,-y+yc);
  glVertex2i(-x+xc, y+yc);
  glVertex2i(-x+xc,-y+yc);
  glEnd();
  glPointSize(pointsize);
}

//--------
static void dibuja(){
  if (!puntos) return;
  int i,j,r,rv,ntris=D.deep, npoints=puntos.deep;
  p2e c,cv;

  // triangulos circulos y voronoi
  for(j=0;j<ntris;j++){
    Dtri &t=D[j]; if (dibujar_c||dibujar_v) t.circulo(c,r);
    // circunferencias
    if (dibujar_c) {
			if ((D.esvirtual(t[0])?1:0)+(D.esvirtual(t[1])?1:0)+(D.esvirtual(t[2])?1:0)==0) {
				glColor3fv(circle_color);
				circulo_Bresenham(c[0],c[1],r);
			}
    }
    if (dibujar_v){
      glColor3fv(voronoi_color);
      // dibuja una linea entre el centro de este triangulo y el de cada vecino
      // para hacerlo solo una vez (no a-b y b-a) va del de mayor puntero al de menor puntero
      // cuando es vecino de la frontera dibuja una linea (larga) perpendicular a la frontera
      Dtri *v;
      i=2; do{
        v=t.vecino[i];
        if(v>&t) continue; // compara punteros para hacer una sola linea por par
        glBegin(GL_LINES);
        glVertex2iv(c);
        if (!v) cv=c+((t[(i+1)%3]-t[(i+2)%3]).giro90())*2; // frontera
        else v->circulo(cv,rv);// hay vecino
        glVertex2iv(cv);
        glEnd();
      } while(i--);
    }
    // triangulos
    if (dibujar_t) {
      glColor3fv(delaunay_color);
      glBegin(GL_LINE_STRIP);
      if (!D.esvirtual(t[0])) glVertex2iv(t[0]);
			if (!D.esvirtual(t[1])) glVertex2iv(t[1]);
			if (!D.esvirtual(t[2])) glVertex2iv(t[2]);
			if (!D.esvirtual(t[0])) glVertex2iv(t[0]);
      glEnd();
    }
  }
  // puntos
  glPointSize(pointsize);
  glColor3fv(vertex_color);
  glBegin(GL_POINTS);
  for(j=0;j<npoints;j++) glVertex2iv(puntos[j]);
  glEnd();
}

//============================================================
// callbacks

//------------------------------------------------------------
// redibuja los objetos
// Cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
  // borra el buffer de pantalla
  glClear(GL_COLOR_BUFFER_BIT);
  dibuja();
  glutSwapBuffers();
  check_gl_error();
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb(int w, int h){
  //if (cl_info) cout << "reshape " << w << "x" << h << endl;

  glViewport(0,0,w,h); // region donde se dibuja

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,h,0,-1,1);

  glutPostRedisplay();
}


//------------------------------------------------------------
// Teclado y Mouse
// GLUT ACTIVE SHIFT Set if the Shift modifier or Caps Lock is active.
// GLUT ACTIVE CTRL  Set if the Ctrl modifier is active.
// GLUT ACTIVE ALT   Set if the Alt modifier is active.

static inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key==27){ // escape => exit
    get_modifiers();
    if (!modifiers) salir();
  }
  else if (key=='i'||key=='I'){ // info
    cl_info=!cl_info;
    cout << ((cl_info)? "Info" : "Sin Info") << endl;
    return;
  }
  else if (key==127){ // del => borra pt
    if (!pt||puntos.deep<=4) return;
    D.quitapunto(*pt);
    puntos.remove(pt);
    delete pt; pt=0;
	  glutPostRedisplay();
    return;
  }
  else if (key=='t'||key=='T'){ // Triangulos
    dibujar_t=!dibujar_t;
    if (cl_info) {
      if (dibujar_t) cout << "Dibuja Triangulos" << endl;
      else cout << "No Dibuja Triangulos" << endl;
    }
	  glutPostRedisplay();
    changemenu=true;
    return;
  }
  else if (key=='c'||key=='C'){ // Circunferencias
    dibujar_c=!dibujar_c;
    if (cl_info) {
      if (dibujar_c) cout << "Dibuja Circunferencias" << endl;
      else cout << "No Dibuja Circunferencias" << endl;
    }
	  glutPostRedisplay();
    changemenu=true;
    return;
  }
  else if (key=='v'||key=='V'){ // Voronoi
    dibujar_v=!dibujar_v;
    if (cl_info) {
      if (dibujar_v) cout << "Dibuja Voronoi" << endl;
      else cout << "No Dibuja Voronoi" << endl;
    }
	  glutPostRedisplay();
    changemenu=true;
    return;
  }
  else if (key=='b'||key=='B'){ // Borra Todo
    while (puntos.deep>4) {
			pt = puntos.pop();
			D.quitapunto(*pt); 
			delete pt; 
		} 
		pt=0;
	  glutPostRedisplay();
    return;
  }
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
void Menu_cb(int value) {
  Keyboard_cb(value);
}

void Menu_changer(int status, int x, int y){
  if (!changemenu) return;
  if (status==GLUT_MENU_IN_USE) 
    return;
  
  if (dibujar_t) glutChangeToMenuEntry(1, "[t] No Dibujar Triangulos", 't');
  else glutChangeToMenuEntry(1, "[t] Dibujar Triangulos", 't');

  if (dibujar_c) glutChangeToMenuEntry(2, "[c] No Dibujar Circunferencias", 'c');
  else glutChangeToMenuEntry(2, "[c] Dibujar Circunferencias", 'c');
  
  if (dibujar_v) glutChangeToMenuEntry(3, "[v] No Dibujar Voronoi", 'v');
  else glutChangeToMenuEntry(3, "[v] Dibujar Voronoi", 'v');
    
  changemenu=false;
}


// Movimientos del mouse

void Passive_Motion_cb(int x, int y){ // cualquier movimiento del mouse
  cout << "\r" << x << ',' << y;
}

void Motion_cb(int x, int y){ // drag
  cout << "\r" << x << ',' << y;
  if (!pt) return;
  p2e pm(x,y);
  if (D.muevepunto(*pt,pm)) glutPostRedisplay();
  else pt=0;
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      // selecciona para mover o borrar
      p2e pm(x,y); pt=0;
      // verifica tolerancia
      int len=puntos.deep;
      while(puntos.deep>4) {
        pt=(p2e*)puntos.pop();
        if (pm.distanciac(*pt)<tol) break;
        pt=0;
      }puntos.deep=len;
      if (!pt) {// agrega // agrega un punto
        pt=new p2e(pm); puntos.push(pt);
        if (D.agregapunto(*pt)) glutPostRedisplay();
        else {puntos.deep--; delete pt; pt=0;}
      }
      // pt es el punto a editar
      glutMotionFunc(Motion_cb); // mueve
      return;
    } // down
    else if (state==GLUT_UP){
      glutMotionFunc(0); // anula el callback para los drags
      return;
    } // up
  } // left
}


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
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);

  glutInitWindowSize(800,600); glutInitWindowPosition(50,50);

  glutCreateWindow("Delaunay"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // resize
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // ctrl alt y shift
  glutMouseFunc(Mouse_cb); // clicks
  glutPassiveMotionFunc(Passive_Motion_cb); // movimientos sin click

  glutCreateMenu(Menu_cb);
    if (dibujar_t)
      glutAddMenuEntry("[t] No Dibujar Triangulos", 't');
    else
      glutAddMenuEntry("[t] Dibujar Triangulos", 't');
    if (dibujar_c)
      glutAddMenuEntry("[c] No Dibujar Circunferencias", 'c');
    else
      glutAddMenuEntry("[c] Dibujar Circunferencias", 'c');
    if (dibujar_v)
      glutAddMenuEntry("[v] No Dibujar Voronoi", 'v');
    else
      glutAddMenuEntry("[v] Dibujar Voronoi", 'v');
      glutAddMenuEntry("---------------------------", 255);
      glutAddMenuEntry("[Del] Borra", 127);
      glutAddMenuEntry("[b] Borra Todo", 'b');
      glutAddMenuEntry("---------------------------", 255);
      glutAddMenuEntry("[i] Info ON/OFF", 'i');
      glutAddMenuEntry("[Esc] Exit", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  glutMenuStatusFunc(Menu_changer);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo
  glDrawBuffer(GL_BACK);
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante
  glShadeModel(GL_FLAT); // no interpola color
  glLineWidth(linewidth);

  // ========================
  // info
  if (cl_info)
    cout << "Vendor:         " << glGetString(GL_VENDOR) << endl
         << "Renderer:       " << glGetString(GL_RENDERER) << endl
         << "GL_Version:     " << glGetString(GL_VERSION) << endl
         << "GL_Extensions:  " << glGetString(GL_EXTENSIONS) << endl
         << "GLU_Version:    " << gluGetString(GLU_VERSION) << endl
         << "GLU_Extensions: " << gluGetString(GLU_EXTENSIONS) << endl
         << _PRINT_INT_VALUE(GL_DOUBLEBUFFER)
         << _PRINT_INT_VALUE(GL_STEREO)
         << _PRINT_INT_VALUE(GL_AUX_BUFFERS)
         << _PRINT_INT_VALUE(GL_RED_BITS)
         << _PRINT_INT_VALUE(GL_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ALPHA_BITS)
         << _PRINT_INT_VALUE(GL_DEPTH_BITS)
         << _PRINT_INT_VALUE(GL_STENCIL_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_RED_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_ALPHA_BITS)
         ;
  // ========================

 
  //inicializa delaunay
  p2e ll(-10000,-10000),ur(20000,20000); // bounding box
  p2e *b0=new p2e(ll),*b1=new p2e(ur,ll),
      *b2=new p2e(ur),*b3=new p2e(ll,ur);
  puntos.push(b0);puntos.push(b1);puntos.push(b2);puntos.push(b3);
  D.init(*b0,*b1,*b2,*b3);
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; //solo para que el builder se quede tranquilo (aqui nunca llega)
}
