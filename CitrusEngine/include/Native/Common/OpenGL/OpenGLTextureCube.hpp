#pragma once

#include "Graphics/Textures/TextureCube.hpp"

#include "glad/gl.h"

namespace CitrusEngine {

    //OpenGL implementation of TextureCube (see TextureCube.hpp for method details)
    class OpenGLTextureCube : public TextureCube {
    public:
        OpenGLTextureCube(std::string posX, std::string negX, std::string posY, std::string negY, std::string posZ, std::string negZ);
        ~OpenGLTextureCube();

        void Bind() override;
        void Unbind() override;
        void Compile() override;
        void Release() override;
    private:
        GLuint compiledForm;

		//Same order as TextureCube::faces
		std::vector<GLenum> faceFormats;
    };
}