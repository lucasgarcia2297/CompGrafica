#include <cstring>
#include "Image.hpp"
#ifdef __APPLE__
# include <OpenGL/glu.h>
# include <GLUT/glext.h>
#else
# include <GL/glu.h>
# include <GL/glext.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image ( ) {
	
}

Image::Image (Image &&other) {
  operator=(std::move(other));
}
Image::Image (const Image & other) {
  operator=(other);
}


Image::Image (const std::string path) {
  buf = stbi_load(path.c_str(), &w, &h, &c, 0);
  if (buf) stbi_buf = true;
}

Image::Image (int w, int h, int c) {
  this->w = w; this->h = h; this->c = c;
  buf = new unsigned char[w*h*c];
  stbi_buf = false;
  std::memset(buf,0,w*c*h);
}

GLuint Image::SetAsTexture (bool generate_mipmaps, bool as_cubemap) const {
  GLuint tex_id;
  glGenTextures(1,&tex_id);
  SetAsTexture(tex_id,generate_mipmaps,as_cubemap);
  return tex_id;
}

void Image::SetAsTexture (GLuint tex_id, bool generate_mipmaps, bool as_cubemap) const {
  glBindTexture(as_cubemap?GL_TEXTURE_CUBE_MAP:GL_TEXTURE_2D, tex_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  int modes[]={0,GL_LUMINANCE,0,GL_RGB,GL_RGBA};
  if (as_cubemap) {
#ifndef __WIN32__
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, generate_mipmaps?GL_TRUE:GL_FALSE);
#endif
    GLenum cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                       GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                       GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                       GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                       GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                       GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };
    
    for(int i=0;i<6;++i) 
      glTexImage2D(cube[i], 0, c, w, h/6, 0, modes[c], GL_UNSIGNED_BYTE, buf+w*h*c/6*i);
  } else if (generate_mipmaps) {
    gluBuild2DMipmaps(GL_TEXTURE_2D, c, w, h,  modes[c], GL_UNSIGNED_BYTE, buf);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, c, w, h, 0, modes[c], GL_UNSIGNED_BYTE, buf);
  }
}

void Image::Reset(bool do_delete) {
  if (do_delete) {
    if (stbi_buf) stbi_image_free(buf);
    else delete [] buf;
  }
  stbi_buf = false; buf = nullptr;
  w = h = c = 0;
}

Image::~Image ( ) {
  Reset();
}

Image &Image::operator=(Image &&other) {
  Reset();
  w = other.w; h = other.h; c = other.c;
  buf = other.buf; stbi_buf = other.stbi_buf;
  other.Reset(false);
  return *this;
}
Image &Image::operator=(const Image & other) {
  Reset();
  w=other.w; c=other.c; h=other.h;
  if (other.buf) {
    buf = new unsigned char[w*h*c];
    std::memcpy(buf,other.buf,w*h*c);
  } else {
    buf = nullptr;
  }
  return *this;
}

void swap(Image & i1, Image & i2) {
  Image ia = std::move(i1);
  i1 = std::move(i2);
  i2 = std::move(ia);
}
