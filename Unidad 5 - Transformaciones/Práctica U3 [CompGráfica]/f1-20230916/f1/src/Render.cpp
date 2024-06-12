#include <glm/ext.hpp>
#include "Render.hpp"
#include "Callbacks.hpp"
#include <glm/ext/vector_float4.hpp>

extern bool wireframe, play, top_view, use_helmet;

// matrices que definen la camara
glm::mat4 projection_matrix, view_matrix;

// función para renderizar cada "parte" del auto

// función que rendiriza todo el auto, parte por parte
void renderPart(const Car &car, const std::vector<Model> &v_models, const glm::mat4 &matrix, Shader &shader) { //matrix es cada parte transformada del autito
	// select a shader
	for(const Model &model : v_models) {
		shader.use();
		// matrixes
		if (play) {
			/// @todo: modificar una de estas matrices para mover todo el auto (todas
			///        las partes) a la posición (y orientación) que le corresponde en la pista
			glm::mat4 matrix2 = glm::mat4(cos(car.ang),	0.0f,	sin(car.ang), 0.0f,
										          0.0f,	1.0f,			0.0f, 0.0f,
										 -sin(car.ang),	0.0f,	cos(car.ang), 0.0f,
										         car.x,	0.0f, 		   car.y, 1.0f ) * matrix ;
			
			shader.setMatrixes(matrix2,view_matrix,projection_matrix);
		} else {
			glm::mat4 model_matrix = glm::rotate(glm::mat4(1.f),view_angle,glm::vec3{1.f,0.f,0.f}) *
				glm::rotate(glm::mat4(1.f),model_angle,glm::vec3{0.f,1.f,0.f}) *
				matrix;
			shader.setMatrixes(model_matrix,view_matrix,projection_matrix);
		}
		
		// setup light and material
		shader.setLight(glm::vec4{20.f,40.f,20.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.35f);
		shader.setMaterial(model.material);
		
		// send geometry
		shader.setBuffers(model.buffers);
		glPolygonMode(GL_FRONT_AND_BACK,(wireframe and (not play))?GL_LINE:GL_FILL);
		model.buffers.draw();
	}
}

// función que actualiza las matrices que definen la cámara
void setViewAndProjectionMatrixes(const Car &car) {
	projection_matrix = glm::perspective( glm::radians(view_fov), float(win_width)/float(win_height), 0.1f, 100.f );
	if (play) {
		if (top_view) {
			/// @todo: modificar el look at para que en esta vista el auto siempre apunte hacia arriba
			glm::vec3 pto_target = {car.x, 0.f, car.y};  //target
			glm::vec3 pto_eye = pto_target+glm::vec3(0.f,30.f,0.f);
			glm::vec3 pto_up = pto_target+glm::vec3(5.f*cos(car.ang),30.f,5.f*sin(car.ang));
			
			
			glm::vec3 up = pto_up - pto_eye;
			view_matrix = glm::lookAt(pto_eye, pto_target, up);
		} else {
			/// @todo: definir view_matrix de modo que la camara persiga al auto desde "atras"
			
			glm::vec3 pto_target = {car.x, 0.f,car.y};
			glm::vec3 pto_eye = pto_target + glm::vec3(-5*cos(car.ang),1.f,-5*sin(car.ang));
			glm::vec3 pto_up = pto_target +glm::vec3(-5*cos(car.ang), 10.f,-5*sin(car.ang));
			
			
			glm::vec3 up = normalize(pto_up-pto_eye);
			
//			glm::vec3 up = glm::vec3(0, 9.5f,0);
			
			glm::vec3 v = normalize(pto_target-pto_eye);
			
			view_matrix = inverse(glm::mat4(glm::vec4(normalize(cross(up,-v)),0.f),glm::vec4(up,0.f),glm::vec4(-v,0.f),normalize(glm::vec4(pto_eye,1.f)))); 								
			view_matrix = glm::lookAt(pto_eye, pto_target,up); 								
		}
	} else {
		view_matrix = glm::lookAt( glm::vec3{0.f,0.f,3.f}, view_target, glm::vec3{0.f,1.f,0.f} );
	}
}

// función que rendiriza todo el auto, parte por parte
void renderCar(const Car &car, const std::vector<Part> &parts, Shader &shader) {
	const Part &axis = parts[0], &body = parts[1], &wheel = parts[2],
		&fwing = parts[3], &rwing = parts[4], &helmet = parts[use_helmet?5:6];
	
	/// @todo: armar la matriz de transformación de cada parte para construir el auto
	//Chasis
	if (body.show or play) {
		glm::mat4 chasis= glm::mat4(1.0f, 0.0f,  0.0f, 0.0f,	//e_x
									0.0f, 1.0f,  0.0f, 0.0f,	//e_y
									0.0f, 0.0f,  1.0f, 0.0f,	//e_z
									0.0f, 0.14f, 0.0f, 1.0f);	//
		renderPart(car,body.models,chasis,shader);
	}
	
	//Ruedas
	if (wheel.show or play) {
		///Rueda delantera derecha.
		//Trasladamos y escalamos.
		glm::mat4 EscTrDelDer = glm::mat4(0.12f, 0.0f, 0.0f, 0.0f,
										  0.0f, 0.12f, 0.0f, 0.0f,
										  0.0f, 0.0f, -0.12f, 0.0f,
										  0.5f, 0.11f, 0.3f, 1.0f);
		
		//Matriz de rotacion. Al doblar, la rueda rota en el eje y.
		glm::mat4 Rot = glm::mat4(cos(car.rang1), 0.0f, -sin(car.rang1), 0.0f,
								  0.0f, 1.0f, 0.0f, 0.0f, //Como rota en el eje y, este eje no varia
								  sin(car.rang1),0.0f,cos(car.rang1),0.0f,
								  0.0f, 0.0f, 0.0f, 1.0f);
		
		//Matriz de rotacion, al acelerar, la rueda rota en el eje z
		glm::mat4 RotAc = glm::mat4(cos(car.rang2), -sin(car.rang2), 0.0f, 0.0f,
									sin(car.rang2), cos(car.rang2), 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f, //Como rota en el eje z, este eje no varia
									0.0f, 0.0f, 0.0f, 1.0f);
		
		glm::mat4 m_rdd = EscTrDelDer*Rot*RotAc;
		
		renderPart(car,wheel.models,m_rdd, shader);
		
		///Rueda delantera izquierda
		//Trasladamos y escalamos.
		glm::mat4 EscTrDelIzq = glm::mat4(0.12f, 0.0f, 0.0f, 0.0f,
										  0.0f, 0.12f, 0.0f, 0.0f,
										  0.0f, 0.0f,  0.12f, 0.0f,
										  0.5f, 0.11f, -0.3f, 1.0f);
		
		//Matriz de rotacion. Al doblar, la rueda rota en el eje y.
		glm::mat4 RotIzq = glm::mat4(cos(car.rang1), 0.0f, sin(car.rang1), 0.0f,
									 0.0f, 1.0f, 0.0f, 0.0f, //Como rota en el eje y, este eje no varia
									 -sin(car.rang1),0.0f,cos(car.rang1),0.0f,
									 0.0f, 0.0f, 0.0f, 1.0f);
		
		//Matriz de rotacion, al acelerar, la rueda rota en el eje z
		glm::mat4 RotAcDelIzq = glm::mat4(cos(car.rang2), -sin(car.rang2), 0.0f, 0.0f,
										  sin(car.rang2), cos(car.rang2), 0.0f, 0.0f,
										  0.0f, 0.0f, 1.0f, 0.0f, //Como rota en el eje z, este eje no varia
										  0.0f, 0.0f, 0.0f, 1.0f);
		
		glm::mat4 m_rdi = EscTrDelIzq*RotIzq*RotAcDelIzq;
		
		renderPart(car,wheel.models,m_rdi,shader);
		
		///Rueda trasera izquierda
		//Trasladamos y escalamos
		glm::mat4 EscTrTrIzq = glm::mat4(0.12f, 0.0f, 0.0f, 0.0f,
										 0.0f, 0.12f, 0.0f, 0.0f,
										 0.0f, 0.0f, 0.12f,0.0f,
										 -0.9f, 0.11f, -0.3f, 1.0f);
		
		glm::mat4 RotAcTrIzq = glm::mat4(cos(car.rang2), -sin(car.rang2), 0.0f, 0.0f,
										 sin(car.rang2), cos(car.rang2), 0.0f, 0.0f,
										 0.0f, 0.0f, 1.0f, 0.0f, //Como rota en el eje z, este eje no varia
										 0.0f, 0.0f, 0.0f, 1.0f);
		
		glm::mat4 m_rti = EscTrTrIzq*RotAcTrIzq;
		
		renderPart(car, wheel.models, m_rti, shader);
		
		///Rueda trasera derecha
		glm::mat4 EscTrTrDer = glm::mat4(0.12f, 0.0f, 0.0f, 0.0f,
										 0.0f, 0.12f, 0.0f, 0.0f,
										 0.0f, 0.0f, -0.13f, 0.0f,
										 -0.9, 0.11f, 0.3f, 1.0f);
		
		glm::mat4 RotAcTrDer = glm::mat4(cos(car.rang2), -sin(car.rang2), 0.0f, 0.0f,
										 sin(car.rang2), cos(car.rang2), 0.0f, 0.0f,
										 0.0f, 0.0f, 1.0f, 0.0f, //Como rota en el eje z, este eje no varia
										 0.0f, 0.0f, 0.0f, 1.0f);
		
		glm::mat4 m_rtd = EscTrTrDer*RotAcTrDer;
		
		renderPart(car, wheel.models, m_rtd, shader);
		 // x4
	}
	
	
	//Alerón delantero
	float scl = 0.30f;
	if (fwing.show or play) {
		glm::mat4 rotacion= glm::mat4( 0.0f, 0.0f,  1.0f, 0.0f,
									   0.0f, 1.0f,  0.0f, 0.0f,
									  -1.0f, 0.0f,  0.0f, 0.0f,
									   0.8f, 0.0f,  0.0f, 1.0f);
		
		glm::mat4 aleron = glm::mat4(1.f*scl, 0.0f,  0.0f, 0.0f,
									 0.0f, 1.f*scl,  0.0f, 0.0f,
									 0.0f, 0.0f,  1.f*scl, 0.0f,
									 0.0f, 0.09f,  0.0f, 1.0f);
		renderPart(car,fwing.models, rotacion * aleron,shader);
	}
	
	//Alerón trasero
	if (rwing.show or play) {
		
		//Traslación y Rotacion 
		glm::mat4 traslacion= glm::mat4(1.0f, 0.0f,   0.f, 0.0f,
									    0.0f, 1.0f,  0.0f, 0.0f,
									     0.f, 0.0f,  1.0f, 0.0f,
									   -0.9f, 0.3f,  0.0f, 1.0f);
		
		float ang = 3.14f/2.f;
		glm::mat4 rotacion= glm::mat4(cos(ang), 0.0f,   sin(ang), 0.0f,
									  0.0f, 1.0f,  0.0f, 0.0f,
									  -sin(ang), 0.0f,  cos(ang), 0.0f,
									 0.0f, 0.0f,  0.0f, 1.0f);
		
		glm::mat4 aleron = glm::mat4( 1.f*scl, 0.0f,  0.0f, 0.0f,
									 0.0f, 1.f*scl,  0.0f, 0.0f,
									 0.0f, 0.0f,  1.f*scl, 0.0f,
									 0.0f, 0.0f,  0.0f, 1.0f);
		
		renderPart(car,rwing.models, traslacion*aleron*rotacion ,shader);
	}
	
	//Casco
	if (helmet.show or play) {
		float ang = 3.14/2.f+car.rang1;
		
		glm::mat4 rotacion= glm::mat4(cos(ang), 0.0f,   sin(ang), 0.0f,
									  0.0f, 1.0f,  0.0f, 0.0f,
									  -sin(ang), 0.0f,  cos(ang), 0.0f,
									  0.0f, 0.0f,  0.0f, 1.0f);
		
		
		glm::mat4 casco= glm::mat4( 0.09f, 0.0f,  0.0f, 0.0f,
								   0.0f, 0.09f,  0.0f, 0.0f,
								   0.0f, 0.0f,  0.09f, 0.0f,
								   0.05f, 0.27f,  0.0f, 1.0f);
		
		renderPart(car,helmet.models,casco*rotacion,shader);
	}
	
	if (axis.show and (not play)) renderPart(car,axis.models,glm::mat4(1.f),shader);
}

// función que renderiza la pista
void renderTrack() {
	static Model track = Model::loadSingle("track",Model::fDontFit);
	static Shader shader("shaders/texture");
	shader.use();
	shader.setMatrixes(glm::mat4(1.f),view_matrix,projection_matrix);
	shader.setMaterial(track.material);
	shader.setBuffers(track.buffers);
	track.texture.bind();
	static float aniso = -1.0f;
	if (aniso<0) glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	track.buffers.draw();
}

void renderShadow(const Car &car, const std::vector<Part> &parts) {
	static Shader shader_shadow("shaders/shadow");
	glEnable(GL_STENCIL_TEST); glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_EQUAL,0,~0); glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	renderCar(car,parts,shader_shadow);
	glDisable(GL_STENCIL_TEST);
}
