#include "Native/Common/OpenGL/OpenGLTexture.h"

#include "stb_image.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include <filesystem>

namespace CitrusEngine {

    Texture* Texture::CreateTextureFromData(unsigned char* dataBuffer, glm::i32vec2 size){
        return new OpenGLTexture(dataBuffer, false, size);
    }

    Texture* Texture::CreateTextureFromFile(std::string filePath){
        //Confirm that provided file path exists
        if(!std::filesystem::exists(filePath) && !std::filesystem::exists(std::filesystem::current_path().string() + "/" + filePath)){
            Asserts::EngineAssert(false, "Cannot load nonexistent texture file \"" + filePath + "\"!");
        }

        int w, h, numChnls;
        unsigned char* dataBuf = stbi_load(filePath.c_str(), &w, &h, &numChnls, 0);

        return new OpenGLTexture(dataBuf, true, { w, h });
    }

    OpenGLTexture::OpenGLTexture(unsigned char* dataBuf, bool dataFromSTB, glm::i32vec2 size){
        compiled = false;
        bound = false;
        dataBuffer = dataBuf;
        bufFromSTB = dataFromSTB;
        imgSize = size;
    }

    OpenGLTexture::~OpenGLTexture(){
        if(bufFromSTB) {
            stbi_image_free(dataBuffer);
        } else {
            delete dataBuffer;
        }
    }

    void OpenGLTexture::Compile(){
        if(compiled){
            Logging::EngineLog(LogLevel::Warn, "Recompiling already compiled texture...");
        }

        Asserts::EngineAssert(dataBuffer, "Cannot compile texture with invalid data buffer!");

        //Create texture
        GLuint tex;
        glGenTextures(1, &tex);

        //Bind texture so we can do stuff to it
        glBindTexture(GL_TEXTURE_2D, tex);

        //Apply texture settings
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Generate OpenGL texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgSize.x, imgSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);
        glGenerateMipmap(GL_TEXTURE_2D);

        //Bind texture again to make sure everything saves
        glBindTexture(GL_TEXTURE_2D, tex);
        
        //Unbind texture to reset texture state
        glBindTexture(GL_TEXTURE_2D, 0);

        compiledForm = tex;
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

        //Clear current program
        glBindTexture(GL_TEXTURE_2D, 0);

        bound = false;
    }

}