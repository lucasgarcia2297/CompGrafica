#ifndef RENDER_HPP
#define RENDER_HPP
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Model.hpp"
#include "Car.hpp"
#include "Shaders.hpp"

// matrices que definen la camara
extern glm::mat4 projection_matrix, view_matrix;

// struct para guardar cada "parte" del auto
struct Part {
	std::string name;
	bool show;
	std::vector<Model> models;
};

// funci�n para renderizar cada "parte" del auto
void renderPart(const Car &car, const std::vector<Model> &v_models, const glm::mat4 &matrix, Shader &shader);

// funci�n que renderiza la sombra sobre la pista
void renderShadow(const Car &car, const std::vector<Part> &parts);

// funci�n que renderiza la pista
void renderTrack();

// funci�n que actualiza las matrices que definen la c�mara
void setViewAndProjectionMatrixes(const Car &car);

// funci�n que rendiriza todo el auto, parte por parte
void renderCar(const Car &car, const std::vector<Part> &parts, Shader &shader);

#endif

