#include "Native/Common/OpenGL/OpenGLTexture.hpp"

#include <filesystem>

#include "Core/Log.hpp"
#include "Core/Assert.hpp"

#include "stb_image.h"

namespace CitrusEngine {

    Texture* Texture::CreateFromFile(std::string filePath){
        Asserts::EngineAssert(std::filesystem::exists(filePath), "Cannot create texture from nonexistent file!");

        int w, h, numChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* imageData = stbi_load(filePath.c_str(), &w, &h, &numChannels, 0);

        Asserts::EngineAssert(imageData, "Image data failed to load from file!");

        return CreateFromData(imageData, { w, h }, numChannels);
    }

    Texture* Texture::CreateFromData(unsigned char* data, glm::ivec2 size, int numChannels) {
        return new OpenGLTexture(data, size, numChannels);
    }

    OpenGLTexture::OpenGLTexture(unsigned char* dataBuf, glm::ivec2 size, int numChannels)
        : compiled(false), bound(false) {
        dataBuffer = dataBuf;
        imgSize = size;
        numImgChannels = numChannels;
    }

    OpenGLTexture::~OpenGLTexture(){
        if(compiled) Release();
        delete dataBuffer;
    }

    void OpenGLTexture::Compile(){
        if(compiled){
            Logging::EngineLog(LogLevel::Warn, "Recompiling already compiled texture...");
        }

        //Generate texture object
        glGenTextures(1, &compiledForm);
        glBindTexture(GL_TEXTURE_2D, compiledForm);

        GLenum format;
        if(numImgChannels == 1) {
            format = GL_RED;
        } else if(numImgChannels == 3) {
            format = GL_RGB;
        } else if(numImgChannels == 4) {
            format = GL_RGBA;
        }

        //Configure texture wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Load texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, imgSize.x, imgSize.y, 0, format, GL_UNSIGNED_BYTE, dataBuffer);
        glGenerateMipmap(GL_TEXTURE_2D);

        compiled = true;
    }

    void OpenGLTexture::Release(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot release uncompiled texture!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot release bound texture!");
            return;
        }
        glDeleteTextures(1, &compiledForm);
        compiled = false;
    }

    void OpenGLTexture::Bind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled texture!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot bind already bound texture!");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, compiledForm);
        bound = true;
    }

    void OpenGLTexture::Unbind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled texture!");
            return;
        }
        if(!bound){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound texture!");
            return;
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        bound = false;
    }
}