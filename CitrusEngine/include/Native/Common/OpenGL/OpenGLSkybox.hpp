#include "Graphics/Skybox.hpp"

namespace CitrusEngine {

	//OpenGL implementation of Skybox (see Skybox.hpp for method details)
	class OpenGLSkybox : public Skybox {
	public:
		OpenGLSkybox(Texture2D* tex, std::vector<glm::vec2> tcs);
		~OpenGLSkybox();

		void Draw() override;
	};
}