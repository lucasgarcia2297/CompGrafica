#include <GL/gl.h>
#include <GL/glut.h>
#include "OSD.hpp"

OSD_t OSD;

void OSD_t::Render (int w, int h) {
  if (text.empty()) return;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);  
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,w,h,0,0,1);
  glDisable(GL_LIGHTING);
  int x = 8, y = 20;
  glRasterPos2i(x,y);
  for(char c:text) {
    if (c=='\n') { glRasterPos2i(x,y+=20); }
    else glutBitmapCharacter(GLUT_BITMAP_9_BY_15,c);
  }
  text.clear();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);  
  glPopMatrix();
  glPopAttrib();
}

