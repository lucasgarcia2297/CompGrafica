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

extern int texmode;
extern bool wire,relleno,cl_info;
extern float line_color[];

//==========================================

void drawColorCube(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glTranslated(-.5,-.5,-.5);

  if (relleno){
    // caras
    glBegin(GL_QUADS);
      //b=0 normal hacia -b
      glTexCoord2d(3.f/4,1.f/3); glColor3f(0,0,0); glVertex3i(0,0,0); 
      glTexCoord2d(3.f/4,2.f/3); glColor3f(0,1,0); glVertex3i(0,1,0);
      glTexCoord2d(4.f/4,2.f/3); glColor3f(1,1,0); glVertex3i(1,1,0);
      glTexCoord2d(4.f/4,1.f/3); glColor3f(1,0,0); glVertex3i(1,0,0);

      //b=1 normal hacia +b
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,0,1); glVertex3i(0,0,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,0,1); glVertex3i(1,0,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,1,1); glVertex3i(1,1,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,1,1); glVertex3i(0,1,1);

      //g=0 normal hacia -g
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,0,0); glVertex3i(0,0,0);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,0,0); glVertex3i(1,0,0);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,0,1); glVertex3i(1,0,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,0,1); glVertex3i(0,0,1);

      //g=1 normal hacia +g
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,1,0); glVertex3i(0,1,0);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,1,1); glVertex3i(0,1,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,1,1); glVertex3i(1,1,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,1,0); glVertex3i(1,1,0);

      //r=0 normal hacia -r
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,0,0); glVertex3i(0,0,0);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,0,1); glVertex3i(0,0,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,1,1); glVertex3i(0,1,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(0,1,0); glVertex3i(0,1,0);

      //r=1 normal hacia +r
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,0,0); glVertex3i(1,0,0);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,1,0); glVertex3i(1,1,0);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,1,1); glVertex3i(1,1,1);
      glTexCoord2d(0.f/4,0.f/3); glColor3f(1,0,1); glVertex3i(1,0,1);
    glEnd();
  }
  glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D);
  if (wire&&!relleno){ // si esta relleno no tiene sentido esto
    // aristas
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

  // ejes
  glBegin(GL_LINES);
    glColor3d(1,0,0); glVertex3d(1.1,0,0); glVertex3d(1.5,0,0);
    glColor3d(0,1,0); glVertex3d(0,1.1,0); glVertex3d(0,1.5,0);
    glColor3d(0,0,1); glVertex3d(0,0,1.1); glVertex3d(0,0,1.5);
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

