#include "OpenGLShader.h"

#include "glad/glad.h"

#include "Core/Log.h"
#include "Core/Assert.h"

namespace CitrusEngine {

    Shader* Shader::CreateShader(std::string vertexSrc, std::string fragmentSrc){
        return new OpenGLShader(vertexSrc, fragmentSrc);
    }

    OpenGLShader::OpenGLShader(std::string vertexSrc, std::string fragmentSrc){
        compiled = false;
        bound = false;
        vertexShader = vertexSrc;
        fragmentShader = fragmentSrc;
    }

    void OpenGLShader::Compile() {
        if(compiled){
            Logging::EngineLog(LogLevel::Warn, "Recompiling already compiled shader...");
        }

        //Create vertex shader base
        GLuint compiledVertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* vertexSrc = vertexShader.c_str();

        //Compile vertex shader
        glShaderSource(compiledVertexShader, 1, &vertexSrc, 0);
        glCompileShader(compiledVertexShader);

        //Confirm vertex shader compilation
        GLint vertexCompileStatus;
        glGetShaderiv(compiledVertexShader, GL_COMPILE_STATUS, &vertexCompileStatus);
        if(vertexCompileStatus == GL_FALSE){
            //Get compilation log
            GLint maxLen = 0;
            glGetShaderiv(compiledVertexShader, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> infoLog(maxLen);
            glGetShaderInfoLog(compiledVertexShader, maxLen, &maxLen, &infoLog[0]);
            //Clean up resources
            glDeleteShader(compiledVertexShader);
            //Log error
            Logging::EngineLog(LogLevel::Error, std::string("Vertex shader compilation failure: ") + infoLog.data());
            return;
        }

        //Create fragment shader base
        GLuint compiledFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fragmentSrc = fragmentShader.c_str();

        //Compile fragment shader
        glShaderSource(compiledFragmentShader, 1, &fragmentSrc, 0);
        glCompileShader(compiledFragmentShader);

        //Confirm fragment shader compilation
        GLint fragmentCompileStatus;
        glGetShaderiv(compiledFragmentShader, GL_COMPILE_STATUS, &fragmentCompileStatus);
        if(fragmentCompileStatus == GL_FALSE){
            //Get compilation log
            GLint maxLen = 0;
            glGetShaderiv(compiledFragmentShader, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> infoLog(maxLen);
            glGetShaderInfoLog(compiledFragmentShader, maxLen, &maxLen, &infoLog[0]);
            //Clean up resources
            glDeleteShader(compiledVertexShader);
            glDeleteShader(compiledFragmentShader);
            //Log error
            Logging::EngineLog(LogLevel::Error, std::string("Fragment shader compilation failure: ") + infoLog.data());
            return;
        }

        //Create shader program
        GLuint program = glCreateProgram();
        
        //Attach compiled shaders to program
        glAttachShader(program, compiledVertexShader);
        glAttachShader(program, compiledFragmentShader);

        //Link shader program
        glLinkProgram(program);

        //Confirm shader program linking
        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if(linkStatus == GL_FALSE){
            //Get linking log
            GLint maxLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> infoLog(maxLen);
            glGetProgramInfoLog(program, maxLen, &maxLen, &infoLog[0]);
            //Clean up resources
            glDeleteProgram(program);
            glDeleteShader(compiledVertexShader);
            glDeleteShader(compiledFragmentShader);
            //Log error
            Logging::EngineLog(LogLevel::Error, std::string("Shader program linking failure: ") + infoLog.data());
            return;
        }

        //Detach and delete linked shaders
        glDetachShader(program, compiledFragmentShader);
        glDetachShader(program, compiledVertexShader);
        glDeleteShader(compiledVertexShader);
        glDeleteShader(compiledFragmentShader);

        compiledForm = program;
        compiled = true;
    }

    void OpenGLShader::Bind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled shader!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot bind already bound shader!");
            return;
        }
        glUseProgram(compiledForm);
        bound = true;
    }

    void OpenGLShader::Unbind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled shader!");
            return;
        }
        if(!bound){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound shader!");
            return;
        }

        //Preserve shader program so it doesn't need to be recompiled
        uint32_t shaderProgram = compiledForm;
        glDeleteProgram(compiledForm);
        compiledForm = shaderProgram;

        bound = false;
    }
}