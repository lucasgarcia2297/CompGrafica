#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glut.h>
#else
# include <GL/gl.h>
# include <GL/glut.h>
#endif
#include "OSD.hpp"

OSD_t OSD;

void OSD_t::Render (int w, int h, bool align_bottom) {
	if (text.empty()) return;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glMatrixMode(GL_MODELVIEW);  
		glPushMatrix();
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);  
			glPushMatrix();
				glLoadIdentity();
				glOrtho(0,w,h,0,0,1);
				glDisable(GL_LIGHTING);
				glDisable(GL_FOG);
				glDisable(GL_DEPTH_TEST);
				int x = 8, y = 20;
				if (align_bottom) {
					int nlines = 0;
					for(char c:text) if (c=='\n') nlines++;
					y = h-20*(nlines+1);
				}
				glRasterPos2i(x,y);
				for(char c:text) {
					if (c=='\n') { 
						glRasterPos2i(x,y+=20); 
					}
					else glutBitmapCharacter(GLUT_BITMAP_9_BY_15,c);
				}
				text.clear();
			glPopMatrix();
		glMatrixMode(GL_MODELVIEW);  
		glPopMatrix();
	glPopAttrib();
}

