#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#ifdef __APPLE__
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif
#include "matrix.hpp"

#define VERSION 20190829


extern float a1, a2, d; // angulos y distancia de la "camara"
extern int win_w, win_h, win_x, win_y; // tamanio y posicion de la ventana
extern float tw,ty,tx, ts, tr, tp, sx, sy; // factores para las tranformaciones
extern bool pective; // proyeccion en perspectiva
extern bool show_w0_plano; // mostrar el plano w=1
extern bool show_w1_plano; // mostrar el plano w=1
extern bool show_w1_ejes; //  mostrar los ejes en w=1
extern bool show_help; // mostrar la ayuda
extern bool show_ejes; // mostrar ejes transformados
extern bool show_full_lines; // mostrar la ayuda
extern bool show_axis_label; // mostrar los rotulos de los ejes
extern bool show_bounding_box; // mostrar el bouding-box del objeto+origen
extern bool perspective; // proyeccion en perspectiva
extern bool quad; // dibujar cuadrilatero o triangulo

// funciones y variables para el calculo
extern matrix mat; // matriz de la transformacion
extern GLfloat pts_bb[][4]; // coordenadas transformadas para dibujar el bounding box
extern GLfloat pts_trans[][4]; // coordenadas transformadas para dibujar el triangulo
extern float pts_tri[][4]; // coordenadas del triangulo original, sin transformar
extern float pts_quad[][4]; // coordenadas del cuadrilatero original, sin transformar
void update_points_and_matrix();
	
// funciones de dibujo
void drawObjects();
void mk_green_plane();

// modos del motion
enum { DM_CAMERA=0, DM_ROTATE_X, DM_ROTATE_W, DM_MOVE_XY, DM_SCALE_W, DM_SCALE_ALL, DM_SCALE_XY, DM_COUNT }; // posible acciones para el drag (motion_cb)
extern const char *help_texts[DM_COUNT]; // textos de ayuda para los modos de drag
extern int def_drag_mode; // modo por defecto para el drag sin shift
extern int cur_drag_mode; // modo actual (puede ser el default o camara si presiono shift)

#endif
