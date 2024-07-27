#include "UI/UIView.hpp"

#include "GLHeaders.hpp"

#include "GLUIView.hpp"
#include "GLUtils.hpp"
#include "Graphics/Window.hpp"

namespace Cacao {
	//Required initialization of static members
	Shader* UIView::shader = nullptr;

	UIView::UIView()
	  : hasRendered(false), bound(false), currentSlot(-1), size(0) {
		//Create buffers
		frontBuffer.reset(new Buffer());
		backBuffer.reset(new Buffer());

		//Create buffer objects
		InvokeGL([this]() {
			std::vector<Buffer> bufs = {this->frontBuffer, this->backBuffer};
			for(std::shared_ptr<Buffer> buf : bufs) {
				//Create framebuffer object
				glGenFramebuffers(1, &buf->fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, &buf->fbo);

				//Create renderbuffer object
				//A sink for depth and stencil data because it doesn't matter but we need to have an output for it
				glGenRenderbuffers(1, &buf->rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, buf->rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 0, 0);

				//Create texture object
				//This is what matters: color output
				glGenTextures(1, &buf->colorTex);
				glBindTexture(GL_TEXTURE_2D, buf->colorTex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				//Attach texture and renderbuffer objects
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf->colorTex, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_RENDERBUFFER, buf->rbo);

				//Confirm framebuffer "completeness"
			CheckException(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, Exception::GetExceptionCodeFromMeaning("GLError"), "UI view framebuffer is not complete!")
				glBindFramebuffer(0);
			}
		}).get();
	}

	UIView::~UIView() {
		//Delete buffer assets
		InvokeGL([this]() {
			glDeleteTextures(1, &this->frontBuffer->colorTex);
			glDeleteTextures(1, &this->backBuffer->colorTex);
			glDeleteRenderbuffers(1, &this->frontBuffer->rbo);
			glDeleteRenderbuffers(1, &this->backBuffer->rbo);
			glDeleteFramebuffers(1, &this->frontBuffer->fbo);
			glDeleteFramebuffers(1, &this->backBuffer->fbo);
		}).wait();
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
		if(size.w != w || size.h != h) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindRenderbuffer(GL_RENDERBUFFER, backBuffer->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

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
			if(kv > furthest) furthest = kv;
		}
		for(int i = furthest; i >= 0; i--) {
			const std::vector<std::shared_ptr<UIRenderable>>& layer = renderables[i];
			for(auto renderable : layer) {
				renderable->Draw();
			}
		}

		//Re-enable depth testing to avoid screwing up global state
		glEnable(GL_DEPTH_TEST);

		//Unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}