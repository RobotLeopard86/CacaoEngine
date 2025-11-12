#pragma once

#include <string>
#include <map>
#include <functional>

#include "Cacao/DllHelper.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/PAL.hpp"

#include "Mesh.hpp"
#include "Tex2D.hpp"
#include "Cubemap.hpp"
#include "GPUManager.hpp"

namespace Cacao {
	class CACAO_API PALModule {
	  public:
		const std::string id;

		virtual void Init() = 0;
		virtual void Term() = 0;
		virtual void Connect() = 0;
		virtual void Disconnect() = 0;
		virtual void Destroy() = 0;
		virtual void SetVSync(bool state) = 0;
		virtual std::unique_ptr<CommandBuffer> CreateCmdBuffer() = 0;

		//==================== IMPL POINTER CONFIGURATION ====================
		virtual Mesh::Impl* ConfigureMesh() = 0;
		virtual Tex2D::Impl* ConfigureTex2D() = 0;
		virtual Cubemap::Impl* ConfigureCubemap() = 0;
		virtual GPUManager::Impl* ConfigureGPUManager() = 0;

		//==================== GPU COMMANDS ====================
		virtual GPUCommand StartRenderingCmd(glm::vec3 clearColor) = 0;
		virtual GPUCommand EndRenderingCmd() = 0;
		virtual GPUCommand PresentCmd() = 0;

		virtual ~PALModule() {}

		bool Initialized() {
			return didInit;
		}
		bool Connected() {
			return connected;
		}

		GPUCommand CommandWithFn(std::function<void(CommandBuffer*)> fn) {
			GPUCommand cmd;
			cmd.apply = std::move(fn);
			return cmd;
		}

	  protected:
		PALModule(const std::string& id)
		  : id(id), didInit(false), connected(false) {}

		bool didInit;
		bool connected;
	};

	struct PAL::Impl {
		std::shared_ptr<PALModule> mod;
		std::map<std::string, std::function<std::shared_ptr<PALModule>()>> registry;
	};
}
