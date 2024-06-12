#include <string>
#include <cstring>
#include "tex_settings.h"

tex_settings_t tex_settings;

static void reload_texture_if_required(const char *filename, bool as_cubemap); 

void tex_settings_t::Apply() const {
  
  glColor3fv(color_material);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glDisable(GL_TEXTURE_2D);
  if (not enable) return;
  
  reload_texture_if_required(filename, cubemap);
  
  auto WHAT = cubemap ? GL_TEXTURE_CUBE_MAP: GL_TEXTURE_2D;
  glEnable(WHAT);
  
  
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,static_cast<int>(tex_env_mode));
  if (tex_env_mode==EnvMode::Blend) 
    glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,tex_env_color);

  glPushMatrix(); glLoadIdentity(); // parece que la matriz afecta a las coords de los planos
  
  glEnable(GL_TEXTURE_GEN_S);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, static_cast<int>(gen_mode_s));
  if (gen_mode_s!=GenMode::SphereMap)
    glTexGenfv(GL_S, gen_mode_s==GenMode::ObjectLinear ? GL_OBJECT_PLANE : GL_EYE_PLANE, plane_s);
  
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, static_cast<int>(gen_mode_t));
  if (gen_mode_t!=GenMode::SphereMap)
    glTexGenfv(GL_T, gen_mode_t==GenMode::ObjectLinear ? GL_OBJECT_PLANE : GL_EYE_PLANE, plane_t);
  
  glPopMatrix();
  
  glTexParameteri(WHAT, GL_TEXTURE_MAG_FILTER, static_cast<int>(mag_filter));
  glTexParameteri(WHAT, GL_TEXTURE_MIN_FILTER, static_cast<int>(min_filter));
  
  glTexParameteri(WHAT, GL_TEXTURE_WRAP_S, static_cast<int>(wrap_s));
  glTexParameteri(WHAT, GL_TEXTURE_WRAP_T, static_cast<int>(wrap_t));
  // estos dos que siguen solo se usan para el cubemap
  glEnable(GL_TEXTURE_GEN_R);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, static_cast<int>(GenMode::ReflectionMap));
  glTexParameteri(WHAT, GL_TEXTURE_WRAP_R, static_cast<int>(wrap_t));
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  
  if (wrap_s==WrapMode::ClampToBorder || wrap_t==WrapMode::ClampToBorder)
    glTexParameterfv(WHAT, GL_TEXTURE_BORDER_COLOR, border_color);
  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
  
  // esta función hace que el color que se mezcla con la textura
  // sea el resultante de la iluminacion ambiente y difusa, excluyendo
  // la especular... la especular se suma después de la aplicación de 
  // textura
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,separate_specular?GL_SEPARATE_SPECULAR_COLOR:GL_SINGLE_COLOR); 
}



bool load_texture(std::string path, bool as_cubemap); // implementado en main

// funcion auxiliar para hacer que recargue la textura solo si cambio el cstring con el nombre del archivo
static void reload_texture_if_required(const char *filename, bool as_cubemap) {
  static char previous_filename[256] = "";
  static bool was_cubemap = false;
  if(std::strcmp(filename,previous_filename) or as_cubemap!=was_cubemap) {
    strcpy(previous_filename,filename);
    was_cubemap = as_cubemap;
    load_texture(filename,as_cubemap);
  }
}
