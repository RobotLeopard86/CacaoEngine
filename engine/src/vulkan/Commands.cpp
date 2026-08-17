#include "Cacao/Exceptions.hpp"
#include "VulkanModule.hpp"
#include "impls/VulkanMaterial.hpp"
#include "impls/VulkanMesh.hpp"
#include "impls/VulkanShader.hpp"
#include "ImplAccessor.hpp"
#include "vulkan/vulkan_enums.hpp"
#include <chrono>

namespace Cacao {
	void VulkanCommandBuffer::StartRendering(glm::vec3 clearColor) {
		//Setup rendering info
		vk::Viewport viewport(0.0f, 0.0f, float(vulkan->swapchain.extent.width), float(vulkan->swapchain.extent.height), 0.0f, 1.0f);
		vk::Rect2D scissor({0, 0}, vulkan->swapchain.extent);
		vk::RenderingAttachmentInfo colorAttachment(vulkan->swapchain.views[render->imageIndex], vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue(std::array<float, 4> {clearColor.r, clearColor.g, clearColor.b, 1.0f}));
		vk::RenderingAttachmentInfo depthAttachment(vulkan->swapchain.depthImages[render->imageIndex].view, vk::ImageLayout::eDepthAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue(1.0f, 0.0f));
		vk::RenderingInfo renderInfo({}, vk::Rect2D({0, 0}, vulkan->swapchain.extent), 1, 0, colorAttachment, &depthAttachment);

		//Make our image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, vulkan->swapchain.images[render->imageIndex],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd.pipelineBarrier2(transition);
		}

		//Make the depth image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, 0, 0, vulkan->swapchain.depthImages[render->imageIndex].obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd.pipelineBarrier2(transition);
		}

		//Configure output region
		cmd.setViewport(0, viewport);
		cmd.setScissor(0, scissor);

		//Begin rendering (this will clear the screen due to values set above)
		cmd.beginRendering(renderInfo);
	}

	void VulkanCommandBuffer::EndRendering() {
		cmd.endRendering();
	}

	void VulkanCommandBuffer::DrawMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform) {
		Check<Mesh, NonexistentValueException>(mesh, "Cannot draw null mesh!");
		Check<Material, NonexistentValueException>(material, "Cannot draw mesh with null material!");
		Check<BadBakeStateException>(mesh->IsBaked(), "Cannot draw unbaked mesh!");
		Check<BadBakeStateException>(material->GetShader()->IsBaked(), "Cannot draw mesh using unbaked shader!");

		//Apply material (will also bind shader pipeline)
		RES_IMPL(Material, Vulkan, *material).Apply(this);

		//Bind mesh vertex and index buffer
		VulkanMeshImpl& vkMesh = RES_IMPL(Mesh, Vulkan, *mesh);
		constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
		cmd.bindVertexBuffers(0, vkMesh.vbo.obj, offsets);
		cmd.bindIndexBuffer(vkMesh.ibo.obj, offsets[0], vk::IndexType::eUint32);

		//Push constant for transform data
		struct PushConstants {
			struct Transform {
				glm::mat4 transform;
				struct _Std140 {
					glm::vec3 col;
					float pad;
				};
				std::array<_Std140, 3> normal;
				float handedness;
			} t;
			struct RMWrap {
				uint32_t mode;
			} r;
		} push = {};
		push.t.transform = transform.GetTransformationMatrix();
		glm::mat3 transformLinear(push.t.transform);
		glm::mat3 normal = glm::transpose(glm::inverse(transformLinear));
		push.t.normal[0].col = normal[0];
		push.t.normal[1].col = normal[1];
		push.t.normal[2].col = normal[2];
		push.t.handedness = (glm::determinant(transformLinear) < 0.0f ? -1.0f : 1.0f);
		push.r.mode = static_cast<uint8_t>(material->GetRenderMode());
		cmd.pushConstants(RES_IMPL(Shader, Vulkan, *material->GetShader()).layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants), &push);

		//Draw mesh
		cmd.drawIndexed(vkMesh.indices.size() * 3, 1, 0, 0, 0);
	}

	void VulkanCommandBuffer::UpdateEngineData(std::shared_ptr<Camera> cam, bool worldRefresh) {
		//Projection correction matrix
		const static glm::mat4 projectionCorrection = glm::transpose(glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.5f,
			0.0f, 0.0f, 0.0f, 1.0f));

		//Camera info
		GlobalsData gd = {};
		gd.viewMatrix = cam->GetViewMatrix();
		gd.projectionMatrix = projectionCorrection * cam->GetProjectionMatrix();
		gd.viewProjectionMatrix = gd.projectionMatrix * gd.viewMatrix;
		gd.camWorldRot = cam->GetRotation();
		gd.camWorldPos = cam->GetPosition();

		//Timing info
		using clock = std::chrono::steady_clock;
		static clock::time_point initial = clock::now();
		static clock::time_point last = clock::now();
		clock::time_point now = clock::now();
		if(worldRefresh) {
			initial = now;
			last = now;
		}
		gd.deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - last).count();
		gd.worldTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - initial).count();
		if(!worldRefresh) last = now;

		//Copy our data to the GPU
		std::memcpy(render->globals.mem, &gd, sizeof(gd));
	}
}