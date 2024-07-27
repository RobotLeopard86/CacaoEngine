#include "UI/UIView.hpp"

#include "GLHeaders.hpp"

#include "GLUIView.hpp"
#include "GLUtils.hpp"
#include "Graphics/Window.hpp"

namespace Cacao {
	//Required initialization of static members
	Shader* UIView::shader = nullptr;

	UIView::UIView()
	  : size(0), bound(false), currentSlot(-1), hasRendered(false) {
		//Create buffers
		frontBuffer.reset(new Buffer());
		backBuffer.reset(new Buffer());

		//Create buffer objects
		InvokeGL([this]() {
			std::vector<std::shared_ptr<Buffer>> bufs = {this->frontBuffer, this->backBuffer};
			for(std::shared_ptr<Buffer> buf : bufs) {
				//Create framebuffer object
				glGenFramebuffers(1, &(buf->fbo));
				glBindFramebuffer(GL_FRAMEBUFFER, buf->rbo);

				//Create color attachment
				glGenTextures(1, &(buf->colorTex));
				glBindTexture(GL_TEXTURE_2D, buf->colorTex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf->colorTex, 0);

				//Create renderbuffer for depth and stencil attachments
				//These are not sampled so we don't use a full texture
				glGenRenderbuffers(1, &(buf->rbo));
				glBindRenderbuffer(GL_RENDERBUFFER, buf->rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1, 1);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buf->rbo);

				//Confirm framebuffer "completeness"
				GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			CheckException(fbStatus == GL_FRAMEBUFFER_COMPLETE, Exception::GetExceptionCodeFromMeaning("GLError"), "UI view framebuffer is not complete!")
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				//Unbind texture and renderbuffer
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}).get();
	}

	UIView::~UIView() {
		//Delete buffer assets
		std::shared_ptr<Buffer> front(frontBuffer), back(backBuffer);
		InvokeGL([front, back]() {
			glDeleteTextures(1, &(front->colorTex));
			glDeleteTextures(1, &(back->colorTex));
			glDeleteRenderbuffers(1, &(front->rbo));
			glDeleteRenderbuffers(1, &(back->rbo));
			glDeleteFramebuffers(1, &(front->fbo));
			glDeleteFramebuffers(1, &(back->fbo));
		});
	}

	void UIView::Bind(int slot) {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot bind UI view in non-rendering thread!")
		CheckException(hasRendered, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind unrendered UI view!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound UI view!");

		//Bind the front buffer to the requested slot
		currentSlot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, frontBuffer->colorTex);
		bound = true;
	}

	void UIView::Unbind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot unbind UI view in non-rendering thread!")
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound UI view!");

		//Unbind the texture from its current slot
		glActiveTexture(GL_TEXTURE0 + currentSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
		currentSlot = -1;
		bound = false;
	}

	void UIView::Draw(std::map<unsigned short, std::vector<std::shared_ptr<UIRenderable>>> renderables) {
		//Regenerate textures and renderbuffers if size has changed
		int w, h;
		glBindTexture(GL_TEXTURE_2D, backBuffer->colorTex);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		if(size.x != w || size.y != h) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindRenderbuffer(GL_RENDERBUFFER, backBuffer->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		//Create projection matrix
		glm::mat4 project = glm::ortho(0.0f, float(size.x), 0.0f, float(size.y));

		//Bind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, backBuffer->fbo);

		//Clear the framebuffer
		//We don't clear the depth buffer because it's irrelevant
		//We use an obnoxious hot pink because it indicates that something is messed up if you can see it
		//and it's different from the 3D clear color
		glClearColor(1.0f, 0.1015625f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		//Render each layer
		int furthest = 0;
		for(const auto& kv : renderables) {
			if(kv.first > furthest) furthest = kv.first;
		}
		for(int i = furthest; i >= 0; i--) {
			const std::vector<std::shared_ptr<UIRenderable>>& layer = renderables[i];
			for(auto renderable : layer) {
				renderable->Draw(project);
			}
		}

		//Re-enable depth testing to avoid screwing up global state
		glEnable(GL_DEPTH_TEST);

		//Unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}