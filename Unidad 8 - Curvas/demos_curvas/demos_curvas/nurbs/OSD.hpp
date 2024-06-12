#ifndef OSD_H
#define OSD_H
#include <string>

/**
* Clase para mostrar texto en la ventana gráfica. 
*
* La idea es usar OSD en lugar de std::cout, de modo que cualquier variable o
* mensaje que se quiera mostrar se vea en la ventana gráfica del programa y no 
* en la consola.
**/

class OSD_t {
public:
  enum mode_t { 
    InmediateMode, // se borra entre frame y frame, hay que rehacerlo siembre
    PersistentMode, // el contenido de un frame se mantiene mientras nadie escriba nada nuevo
    ManualMode // el contenido se mantiene siempre, se acumula todo
  };
  void Render(int w, int h, bool align_bottom=false);
  OSD_t &operator<<(const char c) { text+=c; return *this; }
  OSD_t &operator<<(const char *s) { text+=s; return *this; }
  OSD_t &operator<<(const std::string &s) { text+=s; return *this; }
  OSD_t &operator<<(int x) { text+=std::to_string(x); return *this; }
  OSD_t &operator<<(float x) { return (*this)<<double(x); }
  OSD_t &operator<<(double x) { std::string s = std::to_string(x); return (*this)<<s.substr(0,s.size()-3); }
  void SetMode(mode_t mode) { this->mode = mode; }
  void Clear() { text.clear(); }
private:
  std::string text;
  mode_t mode = InmediateMode;
  size_t last_size = 0; // aux para el PersistentMode
};

extern OSD_t OSD;
#endif

