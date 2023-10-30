#include "Graphics/Skybox.hpp"

namespace CitrusEngine {

	//OpenGL implementation of Skybox (see Skybox.hpp for method details)
	class OpenGLSkybox : public Skybox {
	public:
		OpenGLSkybox(TextureCube tex);
		~OpenGLSkybox();

		void Draw() override;
	};
}