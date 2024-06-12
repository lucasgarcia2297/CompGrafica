#ifndef IMAGE_HPP
#define IMAGE_HPP
#include <string>
#ifdef __APPLE__
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif

class Image {
  unsigned char *buf = nullptr;
  bool stbi_buf = false; // true, al buf lo creo stbi... false, lo hizo esta clase con new
  int w=0, h=0, c=0;
  void Reset(bool do_delete=true);
public:
  Image();
  Image(const std::string path);
  Image(int w, int h, int c=3);
  Image(Image &&other);
  Image(const Image &other);
  Image &operator=(Image &&other);
  Image &operator=(const Image &other);
  bool IsOk() const { return buf!=nullptr; }
  bool HasAlpha() const { return c==4; }
  int Width() const { return w; }
  int Height() const { return h; }
  int Channels() const { return c; }
  unsigned char operator()(int i, int j, int k) const { return buf[(i*w+j)*c+k]; }
  unsigned char &operator()(int i, int j, int k) { return buf[(i*w+j)*c+k]; }
  const unsigned char *data() { return buf; }
  GLuint SetAsTexture(bool generate_mipmaps, bool as_cubemap) const;
  void SetAsTexture(GLuint tex_id, bool generate_mipmaps, bool as_cubemap) const;
  ~Image();
};

void swap(Image &i1, Image &i2);

#endif

