// Aplicación de texturas
// Consigna: Buscar las @@@@ y seguir las instrucciones

#include <cmath> // atan sqrt
#include <cstdlib> // exit
#include <fstream> // file io
#include <cstring>
#include <vector>
#include <algorithm>
#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <GLUT/glut.h>
#else
# ifdef __WIN32__
#   include <GL/glew.h>
# endif
# include <GL/gl.h>
# include <GL/glut.h>
#endif
#include "tex_settings.h"
#include "Image.hpp"
#include "OSD.hpp"

// Sleep, funcion auxiliar para el Idle_cb
#ifdef __WIN32__
# include <windows.h>
#else
# include <thread>
void Sleep(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
#endif


//------------------------------------------------------------
// variables globales y defaults

int
  w=640,h=480, // tamaño de la ventana
  boton=-1, // boton del mouse clickeado
  xclick,yclick, // x e y cuando clickeo un boton
  lod=32; // nivel de detalle (subdivisiones de lineas y superficies parametricas)
float // luces y colores en float
  lpos[]={2,1,5,1}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
  line_color[]={.2f,.3f,.4f,1},
  control_color[]={.5f,.5f,.1f,1}, // poliedros y poligonos de control
  point_color[]={.3f,.3f,.1f,1},   // puntos sueltos o puntos de control
  linewidth=1,pointsize=5, // ancho de lineas y puntos
  escala=150,escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[]={0,0,5}, target[]={0,0,0}, up[]={0,1,0}, // camara, mirando hacia y vertical
  znear=2, zfar=8, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  amy,amy0, // angulo del modelo alrededor del eje y
  ac0,rc0; // angulo resp x y distancia al target de la camara al clickear

bool // variables de estado de este programa
  rota=false,       // gira continuamente los objetos respecto de y
  animado=false,    // el objeto actual es animado
  dibuja=true,      // false si esta minimizado
  wire=false,       // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  smooth=true,      // normales por nodo o por plano
  luz_fija=true,    // luz fija a la camara (o al mundo) 
  blend=false;      // transparencias

short modifiers=0;  // ctrl, alt, shift (de GLUT)
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// temporizador:
static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame

static const double R2G=45/atan(1.0);

GLuint texture_id;

int iarch=11;
static std::vector<std::string> archivos = {
  "bola.png","reflection-sphere.png","boliche.png","arbol.png",
  "cartoon.png","checker.png","disco.png","escher.png","galaxia.png",
  "glare.png","hilit.png","anillo.png","cubemap.png"
};

// Load a PPM Image
bool load_texture(std::string path, bool as_cubemap) {
  if (path.find('.')==std::string::npos) path+=".png"; // si no tiene extension, asumo png
  if (!std::ifstream(path).is_open()) {
    OSD<<"No existe el archivo: " << path << "\n";
    return false; // no existe el archivo ???
  }
  Image img(path); 
  if (!img.IsOk()) {
    OSD<<"Error al leer archivo: " << path << "\n";
    return false;
  }
   // si no tiene alpha, inventarse un alpha trucho
  OSD<<"Textura cargada del archivo: " << path << "\n";
  if (!img.HasAlpha()) {
    int w = img.Width(), h = img.Height();
    Image img_al(w,h,4);
    int c[]={0,1,2}; if (img.Channels()==1) c[1]=c[2]=0; // por si img es solo grises
    for(int i=0;i<h;i++) {
      for(int j=0;j<w;j++) {
        int sum=0;
        for(int k=0;k<3;k++) { 
          sum += (img_al(i,j,k) = img(i,j,c[k]));
        }
        img_al(i,j,3) = 255-sum/3;
      }
    }
    swap(img,img_al);
  }
  img.SetAsTexture(texture_id,true,as_cubemap);
  return true;
}

//------------------------------------------------------------
// redibuja los objetos
extern void drawObjects(int cambia=0);
// Cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
  if (!dibuja) return;
  // borra los buffers de pantalla y z
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  tex_settings.Apply();
  drawObjects();
#ifdef _DEBUG
  // chequea errores
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
    std::cerr << "ERROR: ";
    if      (errornum==GL_INVALID_ENUM)      std::cerr << "GL_INVALID_ENUM" << std::endl;
    else if (errornum==GL_INVALID_VALUE)     std::cerr << "GL_INVALID_VALUE" << std::endl;
    else if (errornum==GL_INVALID_OPERATION) std::cerr << "GL_INVALID_OPERATION" << std::endl;
    else if (errornum==GL_STACK_OVERFLOW)    std::cerr << "GL_STACK_OVERFLOW" << std::endl;
    else if (errornum==GL_STACK_UNDERFLOW)   std::cerr << "GL_STACK_UNDERFLOW" << std::endl;
    else if (errornum==GL_OUT_OF_MEMORY)     std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
    errornum=glGetError();
  }
#endif // _DEBUG

  glColor3f(0,0,0);
  OSD.Render(w,h,true);
  glutSwapBuffers();
}

//------------------------------------------------------------
// Regenera la matriz de proyeccion
// cuando cambia algun parametro de la vista
void regen() {
  if (!dibuja) return;

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();

  double w0=(double)w/2/escala,h0=(double)h/2/escala; // semiancho y semialto en el target

  // frustum
  double // "medio" al cuete porque aqui la distancia es siempre 5
    delta[3]={(target[0]-eye[0]), // vector ojo-target
              (target[1]-eye[1]),
              (target[2]-eye[2])},
    dist=sqrt(delta[0]*delta[0]+
              delta[1]*delta[1]+
              delta[2]*delta[2]);
  w0*=znear/dist,h0*=znear/dist;  // w0 y h0 en el near
  glFrustum(-w0,w0,-h0,h0,znear,zfar);
  
  // ortogonal
//  glOrtho(-w0,w0,-h0,h0,znear,zfar);

  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // matriz del modelo->view

  if (luz_fija) glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz
  gluLookAt(   eye[0],   eye[1],   eye[2],
            target[0],target[1],target[2],
                up[0],    up[1],    up[2]);// ubica la camara

  glRotatef(amy,0,1,0); // rota los objetos alrededor de y

  if (!luz_fija) glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

  glutPostRedisplay(); // avisa que hay que redibujar
}

//------------------------------------------------------------
// Animacion

// El "framerate" real (cuadros por segundo)
// depende de la complejidad del modelo (lod) y la aceleracion por hardware
void Idle_cb() {
  // Cuenta el lapso de tiempo
  static int anterior=glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
  int tiempo=glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
  if (lapso<msecs) Sleep(msecs-lapso); // ademas de controlar la velocidad, el sleep evita la busy-wait (freir el micro esperando)
  anterior=tiempo;
  if (rota) { amy+=2; if (amy>360) amy-=360; regen(); }
  glutPostRedisplay(); // redibuja
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  h=height; w=width;
//  OSD << "reshape: " << w << "x" << h << "\n";

  if (w==0||h==0) {// minimiza
    dibuja=false; // no dibuja mas
    glutIdleFunc(0); // no llama a cada rato a esa funcion
    return;
  }
  else if (!dibuja&&w&&h){// des-minimiza
    dibuja=true; // ahora si dibuja
    glutIdleFunc(Idle_cb); // registra de nuevo el callback
  }

  glViewport(0,0,w,h); // region donde se dibuja

  regen(); //regenera la matriz de proyeccion
  glutPostRedisplay();
}


//------------------------------------------------------------
// Movimientos del mouse
void Motion_cb(int xm, int ym){ // drag
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
      regen();
    }
    else { // manipulacion
      double yc=eye[1]-target[1],zc=eye[2]-target[2];
      double ac=ac0+(ym-yclick)*720.0/h/R2G;
      yc=rc0*sin(ac); zc=rc0*cos(ac);
      up[1]=zc; up[2]=-yc;  // perpendicular
      eye[1]=target[1]+yc; eye[2]=target[2]+zc;
      amy=amy0+(xm-xclick)*180.0/w;
      regen();
    }
  }
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  static bool old_rota=false;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      xclick=x; yclick=y;
      boton=button;
      old_rota=rota; rota=false;
      get_modifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
      if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
        escala0=escala;
      }
      else { // manipulacion
        double yc=eye[1]-target[1],zc=eye[2]-target[2];
        rc0=sqrt(yc*yc+zc*zc); ac0=atan2(yc,zc);
        amy0=amy;
      }
    }
    else if (state==GLUT_UP){
      rota=old_rota;
      boton=-1;
      glutMotionFunc(0); // anula el callback para los drags
    }
  }
}
//------------------------------------------------------------
// Teclado
// Special keys (non-ASCII)
// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT)
      exit(EXIT_SUCCESS);
  }
  if (key==GLUT_KEY_F5){ // relee textura
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_SHIFT)
      iarch=(iarch-1+archivos.size())%archivos.size();
    else
      iarch=(iarch+1)%archivos.size();
    std::strcpy(tex_settings.filename,archivos[iarch].c_str());
    tex_settings.cubemap = archivos[iarch]=="cubemap.png";
    glutPostRedisplay();
  }
  if (key==GLUT_KEY_UP||key==GLUT_KEY_DOWN){ // camara
    // la camara gira alrededor del eje -x
    double yc=eye[1]-target[1],zc=eye[2]-target[2],
           rc=sqrt(yc*yc+zc*zc),ac=atan2(yc,zc);
    ac+=((key==GLUT_KEY_UP) ? 2 : -2)/R2G;
    yc=rc*sin(ac); zc=rc*cos(ac);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    regen();
  }
  if (key==GLUT_KEY_LEFT){ // gira
    amy-=1;
    regen();
  }
  if (key==GLUT_KEY_RIGHT){ // gira
    amy+=1;
    regen();
  }
  if (key==GLUT_KEY_PAGE_UP||key==GLUT_KEY_PAGE_DOWN){ // velocidad
    if (key==GLUT_KEY_PAGE_DOWN) ms_i++;
    else ms_i--;
    if (ms_i<0) ms_i=0; if (ms_i==ms_n) ms_i--;
    msecs=ms_lista[ms_i];
    if (msecs<1000)
      OSD << 1000/msecs << "fps" << "\n";
    else
      OSD << msecs/1000 << "s/frame)" << "\n";
  }
}

template<typename T>
using EnumMap = std::vector<std::pair<T,std::string>>;

// avanza t a otro valor del enum, y retorna el string ese nuevo valor
template<typename T>
std::string ciclar(T &t, const EnumMap<T> &v, int d) {
  auto p = v.begin(); while(p!=v.end() && p->first!=t) ++p;
  int i = std::distance(v.begin(),p);
  t = v[(i+v.size()+d)%v.size()].first;
  return v[(i+v.size()+d)%v.size()].second;
}

// Maneja pulsaciones del teclado (ASCII keys)
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
    case '\r': OSD.Clear(); break;
    case 'b': case 'B': // modo de aplicacion de textura
      {
        static EnumMap<EnvMode> v = {
          { EnvMode::Add,      "Add" },
          { EnvMode::Modulate, "Modulate" },
          { EnvMode::Decal,    "Decal" },
          { EnvMode::Blend,    "Blend" },
          { EnvMode::Replace,  "Replace" }
        };
        std::string str = ciclar( tex_settings.tex_env_mode, v , key=='b'?+1:-1 );
        OSD << "Modo de aplicacion: " << str << "\n";
      } break;
    case 'c': case 'C': // cambia
      drawObjects((key=='c') ? 1 : -1);
      break;
    case 'f': case 'F': // relleno
      relleno=!relleno;
      OSD << ((relleno)? "Relleno" : "Sin Relleno") << "\n";
      break;
    case 'w': case 'W': // wire
      wire=!wire;
      OSD << ((wire)? "Lineas" : "Sin Lineas") << "\n";
      break;
    case 'n': case 'N': // filtro de minificacion
      {
        static EnumMap<MinFilter> v = { 
          { MinFilter::Nearest,              "Nearest" }, 
          { MinFilter::Linear,               "Linear" }, 
          { MinFilter::NearestMipmapNearest, "Nearest Mipmap-Nearest" }, 
          { MinFilter::NearestMipmapLinear,  "Nearest Mipmap-Linear" }, 
          { MinFilter::LinearMipmapNearest,  "Linear Mipmap-Nearest" }, 
          { MinFilter::LinearMipmapLinear,   "Linear Mipmap-Linear" }
        };
        std::string str = ciclar(tex_settings.min_filter, v, key=='n' ? +1 : -1 );
        OSD << "Filtro de minificacion: " << str << "\n";
      } break;
    case 'l': case 'L': // posicion de la luz
      luz_fija = !luz_fija;
      OSD << "Luz fija a la camara: " << (luz_fija?"Si":"No") << "\n";
      regen();
      break;
    case 'm': case 'M': // filtro de minificacion
      {
        static EnumMap<MagFilter> v = { 
          { MagFilter::Nearest, "Nearest" },
          { MagFilter::Linear,  "Linear" }
        };
        std::string str = ciclar(tex_settings.mag_filter, v, key=='n' ? +1 : -1 );
        OSD << "Filtro de magnificacion: " << str << "\n";
      } break;
    case 'r': case 'R': // rotacion
      rota=!rota;
      OSD << ((rota)? "Gira" : "No Gira") << "\n";
      break;
    case 'g': case 'G': // smooth
      smooth=!smooth;
      glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);
      OSD << ((smooth)? "Suave" : "Facetado") << "\n";
      break;
    case 'a': case 'A': // transparencia
      blend=!blend;
      if (blend){
        glEnable(GL_BLEND);
        OSD << "Transparencia" << "\n";
      }
      else {
        glDisable(GL_BLEND);
        OSD << "Sin Transparencia" << "\n";
      }
      break;
    case 's': case 'S':  // Clamp/Repeat
      {
        static EnumMap<WrapMode> v = { 
          { WrapMode::ClampToEdge,    "Clamp To Edge" }, 
          { WrapMode::ClampToBorder,  "Clamp To Border" }, 
          { WrapMode::Repeat,         "Repeat" }, 
          { WrapMode::MirroredRepeat, "Mirrored Repeat" }
        };
        std::string str = ciclar(tex_settings.wrap_s, v, key=='s' ? +1 : -1 );
        OSD << "Modo de repeticion en S: " << str << "\n";
      } break;
    case 't': case 'T':  // Clamp/Repeat
      {
        static EnumMap<WrapMode> v = { 
          { WrapMode::ClampToEdge,    "Clamp To Edge" }, 
          { WrapMode::ClampToBorder,  "Clamp To Border" }, 
          { WrapMode::Repeat,         "Repeat" }, 
          { WrapMode::MirroredRepeat, "Mirrored Repeat" }
        };
        std::string str = ciclar(tex_settings.wrap_t, v,  key=='t' ? +1 : -1 );
        OSD << "Modo de repeticion en T: " << str << "\n";
      } break;
    case 'x': case 'X': // modo de generacion de textura
      {
        static EnumMap<GenMode> v = { 
          { GenMode::ObjectLinear, "Object Linear" }, 
          { GenMode::EyeLinear,    "Eye Linear" }, 
          { GenMode::SphereMap,    "Sphere Map" }
        };
        std::string str = ciclar(tex_settings.gen_mode_s, v, key=='x' ? +1 : -1);
        OSD << "Modo de generación para S: " << str << "\n";
      } break;
    case 'y': case 'Y': // modo de generacion de textura
      {
        static EnumMap<GenMode> v = { 
          { GenMode::ObjectLinear, "Object Linear" }, 
          { GenMode::EyeLinear,    "Eye Linear" }, 
          { GenMode::SphereMap,    "Sphere Map" }
        };
        std::string str = ciclar(tex_settings.gen_mode_t, v, key=='y' ? +1 : -1);
        OSD << "Modo de generación para T: " << str << "\n";
      } break;
    case 'q': // escape => exit
      get_modifiers();
      if (!modifiers)
        exit(EXIT_SUCCESS);
      break;
    case '+': case '-': // lod
      if (key=='+') lod++; else {lod--; if (lod==0) lod=1;}
      OSD << "Nivel de detalle: "  << lod << "\n";
      break;
  }
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  if (value<256) Keyboard_cb(value);
  else Special_cb(value-256);
}
//------------------------------------------------------------
// pregunta a OpenGL por el valor de una variable de estado
int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<"\n"

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  
  OSD.SetMode(OSD_t::PersistentMode);
  
  // pide z-buffer, color RGBA y double buffering
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);

  glutCreateWindow("Aplicacion de Texturas"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // teclas especiales
  glutMouseFunc(Mouse_cb); // botones picados
  glutIdleFunc(Idle_cb); // registra el callback

  
  OSD<<"Open GL Version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << "\n";
  
  // ========================
  // estado normal del OpenGL
  // ========================

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_NORMALIZE); // para que el scaling no moleste
  glDisable(GL_AUTO_NORMAL); // para nurbs??
  glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1); // coplanaridad

  // interpola normales por nodos o una normal por plano
  glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);

  // ancho de lineas y puntos
  glLineWidth(linewidth); glPointSize(pointsize);

  // antialiasing
//  glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);

  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

  // transparencias
  if (blend) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  // color de fondo
  glClearColor(1.f,1.f,1.f,0);

  // direccion de los poligonos
  glFrontFace(GL_CCW); glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glCullFace(GL_BACK); // se habilita por objeto (cerrado o abierto)

  // define luces
  static const float
    lambient[]={.1f,.1f,.1f,1}, // luz ambiente
    ldiffuse[]={.65f,.65f,.65f,1}, // luz difusa
    lspecular[]={1,1,1,1};      // luz especular
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  glLightfv(GL_LIGHT0,GL_SPECULAR,lspecular);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  
  // material estandar
  static const float
    fambient[]={.4f,.2f,.1f,1},
    fdiffuse[]={.5f,.2f,.1f,1},
    fspecular[]={1,1,1,1},
    bambient[]={.2f,.4f,.4f,1},
    bdiffuse[]={.2f,.5f,.4f,1},
    bspecular[]={1,1,1,1};
  static const int
    fshininess=75,
    bshininess=100;
  glMaterialfv(GL_FRONT,GL_AMBIENT,fambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,fdiffuse);
  glMaterialfv(GL_FRONT,GL_SPECULAR,fspecular);
  glMateriali(GL_FRONT,GL_SHININESS,fshininess);
  glMaterialfv(GL_BACK,GL_AMBIENT,bambient);
  glMaterialfv(GL_BACK,GL_DIFFUSE,bdiffuse);
  glMaterialfv(GL_BACK,GL_SPECULAR,bspecular);
  glMateriali(GL_BACK,GL_SHININESS,bshininess);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

  // textura
  glGenTextures(1, &texture_id);
  std::strcpy(tex_settings.filename,archivos[iarch].c_str());
  //Fijar aqui el modo seleccionado para generacion automatica
//  tex_settings.Apply();
  
  // ========================
  // crea el menu
  glutCreateMenu(Menu_cb);
  glutAddMenuEntry("    [F5]_Cambia textura        ", (256+GLUT_KEY_F5));
  glutAddMenuEntry("     [b]_Modo Aplicacion       ", 'b');
  glutAddMenuEntry("     [a]_Transparencia         ", 'a');
  glutAddMenuEntry("     [s]_Clamp/Repeat S        ", 's');
  glutAddMenuEntry("     [t]_Clamp/Repeat t        ", 't');
  glutAddMenuEntry("     [x]_Modo Generacion S     ", 'x');
  glutAddMenuEntry("     [y]_Modo Generacion T     ", 'y');
  glutAddMenuEntry("     [c]_Cambia Objeto         ", 'c');
  glutAddMenuEntry("     [f]_Caras Rellenas        ", 'f');
  glutAddMenuEntry("     [l]_Luz fija/movil        ", 'l');
  glutAddMenuEntry("     [w]_Wireframe             ", 'w');
  glutAddMenuEntry("     [n]_Filtro Minificacion   ", 'n');
  glutAddMenuEntry("     [m]_Filtro Magnificacion  ", 'm');
  glutAddMenuEntry("     [r]_Rota                  ", 'r');
  glutAddMenuEntry("     [g]_Flat/Gouraud          ", 'g');
  glutAddMenuEntry("     [+]_+Nivel de Detalle     ", '+');
  glutAddMenuEntry("     [-]_-Nivel de Detalle     ", '-');
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  glTexGenfv(GL_S, GL_OBJECT_PLANE, tex_settings.plane_s);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, tex_settings.plane_t);
  glTexGenfv(GL_S, GL_EYE_PLANE,    tex_settings.plane_s);
  glTexGenfv(GL_T, GL_EYE_PLANE,    tex_settings.plane_t);
  
  regen(); // para que setee las matrices antes del 1er draw
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
