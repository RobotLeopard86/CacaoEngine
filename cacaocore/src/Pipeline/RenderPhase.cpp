#include "Pipeline/RenderPhase.hpp"
#include "Graphics/Window.hpp"

namespace Cacao {
	//Required static variable initialization
	RenderPhase* RenderPhase::instance = nullptr;
	bool RenderPhase::instanceExists = false;

	//Singleton accessor
	RenderPhase* RenderPhase::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new RenderPhase();
			instanceExists = true;
		}

		return instance;
	}

	void RenderPhase::_Run(){
		if(!renderQueue.empty()){
			//Get next job
			RenderJob job = renderQueue.front();
			renderQueue.pop();

			//Execute job commands
			for(RenderCmd cmd : job.renderCmds){
				//Validate that this command is good to go
				if(!cmd.material.shader->IsCompiled()) continue;
				if(!cmd.mesh->IsCompiled()) continue;

				//Execute the command
				ExecuteRenderCmd(cmd);
			}
		}
		//Update window
		Window::GetInstance()->Update();
	}
}
