#ifndef TEX_SETTINGS_H
#define TEX_SETTINGS_H

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

// modos de generación automática de las coordenadas de textura
enum class GenMode:int { 
  ObjectLinear = GL_OBJECT_LINEAR,  // plano, definido en espacio del modelo
  EyeLinear = GL_EYE_LINEAR,        // plano, definido en espacio del ojo
  SphereMap  = GL_SPHERE_MAP,       // esférico (para simular reflexiones,
                                    //   probar con las texturas que son "redondas")
  ReflectionMap = GL_REFLECTION_MAP // para usar solamente en combinacion con 
                                    //   cubemap activado y la textura cubemap.ong
};

// modos de aplicación (mezcla entre textura y material)
enum class EnvMode:int { 
  Add = GL_ADD,           // sumar ambos colores
  Modulate = GL_MODULATE, // multiplicar ambos colores
  Decal = GL_DECAL,       // reemplazar segun alpha (alpha=0->material,
                          //                         alpha=1->textura)
  Replace = GL_REPLACE,   // reemplazar completo (usar el color de la textura, 
                          //                      ignorar el material)
  Blend = GL_BLEND        // mezclar color del material con un color fijo
                          //    (tex_env_color) usando la textura como peso
};

// filtros para magnificacion
enum class MagFilter:int { 
  Nearest = GL_NEAREST, // textel más cercano
  Linear  = GL_LINEAR   // bilineal entre los 4 más cercanos
};

// filtros para minificacion
enum class MinFilter:int { 
  Nearest = GL_NEAREST, // textel más cercano
  Linear = GL_LINEAR,   // bilineal entre los 4 más cercanos
  // combinaciones de los anteriores con los niveles de mipmaps
  NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST, 
  NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR, 
  LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
  LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

// modo de repetición (cuando s/t sale de [0,1])
enum class WrapMode:int {
  ClampToEdge = GL_CLAMP,              // "clampear, si <0 -> 0, si >1 -> 1
  ClampToBorder = GL_CLAMP_TO_BORDER,  // usar color de borde cuando sale de [0,1]
  Repeat = GL_REPEAT,                  // repetir infinitamente
  MirroredRepeat  = GL_MIRRORED_REPEAT // repetir espejando copia por medio
};

/// Este es el struct que guarda toda la configuración 
/// para la aplicación de la textura
struct tex_settings_t {
  
  // color para las comps difusa y ambiente del material (phong)
  float color_material[4] = { .5f,.2f,.1f };
  
  // habilitar aplicacion de textura
  bool enable = true;
  
  // nombre del archivo con la imagen (ver los que hay en la carpeta "bin")
  char filename[256];
  // cargar como cubemap (cuando la imagen es una tira con las 6 caras del cubo)
  bool cubemap = false;
  
  // modo de aplicación
  EnvMode tex_env_mode = EnvMode::Replace;
  // color fijo para el modo Blend
  float tex_env_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
  
  // filtros de minificacion/magnificacion
  MinFilter min_filter = MinFilter::Nearest;
  MagFilter mag_filter = MagFilter::Nearest;
  // cantidad de muestras para filtrado anisotropico (1 seria como "desactivado")
  // el máximo puede variar de acuerdo a la gpu (8 o 16 son un valores habituales)
  float anisotropy = 1.f;
  
  // modos de repeticion
  WrapMode wrap_s = WrapMode::Repeat;
  WrapMode wrap_t = WrapMode::Repeat;
  // color de borde para el modo clamp_to_border
  float border_color[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
  
  // modos de generacion de las coordenadas de textura
  GenMode gen_mode_s = GenMode::ObjectLinear;
  GenMode gen_mode_t = GenMode::ObjectLinear;
  // coeficientes de los planos para los modos ObjectLinear y EyeLinear
  float plane_s[4] = {1.f,0.f,0.f,0.f};
  float plane_t[4] = {0.f,1.f,0.f,0.f};
  
  // al activarlo se usan al mezclar ilumnacion y material solo las comps.
  // difusa y ambiente, y sumar; la especular se suma luego de le mezcla
  bool separate_specular = false;
  
  void Apply() const;
  
};

extern tex_settings_t tex_settings;

#endif

