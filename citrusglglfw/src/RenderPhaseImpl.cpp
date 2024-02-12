#include "Pipeline/RenderPhase.hpp"
#include "GLRenderPhaseData.hpp"

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"

//For my sanity
#define nd ((GLRenderPhaseData*)nativeData)

namespace Citrus {
	RenderPhase::RenderPhase() {
		nativeData = new GLRenderPhaseData();
		//Set up uniform buffers
		glGenBuffers(1, &(nd->ubo));
		glBindBuffer(GL_UNIFORM_BUFFER, nd->ubo);
		glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, nd->ubo, 0, 3 * sizeof(glm::mat4));
	}

	void RenderPhase::BeforeShutdown() {
		glDeleteBuffers(1, &(nd->ubo));
	}

	void RenderPhase::Prerender(glm::mat4 pm, glm::mat4 vm){
		//Update projection and view matrices
		glBindBuffer(GL_UNIFORM_BUFFER, nd->ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr()); //p
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr()); //v
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	};

	void RenderPhase::ExecRenderCmd(RenderCmd cmd) {

	}
}