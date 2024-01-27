#include "Graphics/Skybox.hpp"

#include "glad/gl.h"

namespace CitrusEngine {

	//OpenGL implementation of Skybox (see Skybox.hpp for method details)
	class OpenGLSkybox : public Skybox {
	public:
		OpenGLSkybox(Cubemap* tex);
		~OpenGLSkybox();

		void Draw() override;
	private:
		//Skybox vertices
		static float skyboxVerts[];

		//OpenGL skybox resources
		GLuint vao, vbo;
	};
}