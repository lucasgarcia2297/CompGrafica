#include "globals.hpp"

float a1=0.1, a2=.6, d=5; // angulos y distancia de la "camara"
int win_w=640, win_h=480, win_x=100, win_y=100; // tamanio y posicion de la ventana

float tw=1,ty=0,tx=0, ts=1, tr=0, tp=0, sx=1, sy=1; // factores para las tranformaciones
bool show_bounding_box=false; // mostrar el bouding-box del objeto+origen
bool show_help=false; // mostrar la ayuda
bool show_w0_plano=true; // mostrar el plano w=0
bool show_w1_plano=true; // mostrar el plano w=1
bool show_w1_ejes=false; // mostrar el los ejes del plano w=1
bool show_ejes=true; // mostrar los ejes transformados
bool show_full_lines=true; // mostrar el plano w=1
bool show_axis_label=true; // mostrar los rotulos de los ejes
bool perspective=true; // proyeccion en perspectiva
bool quad=false; // dibujar cuadrilatero o triangulo

const char *help_texts[DM_COUNT]= { // textos de ayuda para los modos de drag
	"Shift+arrastrar = Mover Camara",
	"Arrastrar = Rotar sobre el eje X",
	"Arrastrar = Rotar sobre el eje W",
	"Arrastrar = Shear sobre los ejes X e Y",
	"Arrastrar = Escalar eje W",
	"Arrastrar = Escalar uniforme",
	"Arrastrar = Escalar plano XY"
};
int def_drag_mode=DM_CAMERA; // modo por defecto para el drag sin shift
int cur_drag_mode=DM_CAMERA; // modo actual (puede ser el default o camara si presiono shift)


matrix mat; // matrices para el calculo de las tranformaciones

// los vectores son 4d para aprovechar el w de la 4 dimension para hacer la proyeccion sobre w=1 en 3
// o sea, para analizar las transformaciones no considerar la cuarta coord, solo se usa para dibujar
GLfloat pts_trans[8][4]; // coordenadas transformadas para dibujar
GLfloat pts_bb[8][4]; // coordenadas transformadas para dibujar

float pts_tri[][4]= { // coordenadas del triangulo original, sin transformar
	{0,1,2,0}, {-1,-1,2,0}, {1,-1,2,0},
};
float pts_quad[][4]= { // coordenadas del cuadrilatero original, sin transformar
	{-.8,1.2,2,0}, {-.8,-1.2,2,0}, {.8,-1.2,2,0}, {.8,1.2,2,0},
};
