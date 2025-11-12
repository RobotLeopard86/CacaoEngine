#include "Cacao/GPU.hpp"
#include "Context.hpp"
#include "OpenGLModule.hpp"

#include "glad/gl.h"

namespace Cacao {
	GPUCommand OpenGLModule::StartRenderingCmd(glm::vec3 clearColor) {
		return CommandWithFn([clearColor](CommandBuffer* cmd) {
			//Make sure this is an OpenGL buffer
			OpenGLCommandBuffer* glCmd = [&cmd]() -> OpenGLCommandBuffer* {
				if(OpenGLCommandBuffer* glcb = dynamic_cast<OpenGLCommandBuffer*>(cmd)) {
					return glcb;
				} else {
					Check<BadTypeException>(false, "Cannot submit a non-OpenGL command buffer to the OpenGL backend!");
					throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
				}
			}();

			//Add task to command buffer
			glCmd->AddTask([clearColor]() {
				//Clear screen
				glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			});
		});
	}

	GPUCommand OpenGLModule::EndRenderingCmd() {
		//OpenGL has no formal "end rendering" system
		return CommandWithFn([](CommandBuffer*) {});
	}

	GPUCommand OpenGLModule::PresentCmd() {
		return CommandWithFn([](CommandBuffer* cmd) {
			//Make sure this is an OpenGL buffer
			OpenGLCommandBuffer* glCmd = [&cmd]() -> OpenGLCommandBuffer* {
				if(OpenGLCommandBuffer* glcb = dynamic_cast<OpenGLCommandBuffer*>(cmd)) {
					return glcb;
				} else {
					Check<BadTypeException>(false, "Cannot submit a non-OpenGL command buffer to the OpenGL backend!");
					throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
				}
			}();

			//Add task to command buffer
			glCmd->AddTask([]() {
				ctx->SwapBuffers();
			});
		});
	}
}