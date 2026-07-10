#include "Cacao/Exceptions.hpp"
#include "VulkanModule.hpp"
#include "impls/VulkanMaterial.hpp"
#include "impls/VulkanMesh.hpp"
#include "impls/VulkanShader.hpp"
#include "ImplAccessor.hpp"
#include "vulkan/vulkan_enums.hpp"

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
		RES_IMPL(Material, Vulkan, *material).Upload(this);

		//Bind mesh vertex and index buffer
		VulkanMeshImpl& vkMesh = RES_IMPL(Mesh, Vulkan, *mesh);
		constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
		cmd.bindVertexBuffers(0, vkMesh.vbo.obj, offsets);
		cmd.bindIndexBuffer(vkMesh.ibo.obj, offsets[0], vk::IndexType::eUint32);

		//Push constant for transform data
		struct PushConstants {
			glm::mat4 transform;
			struct _Std140 {
				glm::vec3 col;
				float pad;
			};
			std::array<_Std140, 3> normal;
			float handedness;
		} push = {};
		push.transform = transform.GetTransformationMatrix();
		glm::mat3 transformLinear(push.transform);
		glm::mat3 normal = glm::transpose(glm::inverse(transformLinear));
		push.normal[0].col = normal[0];
		push.normal[1].col = normal[1];
		push.normal[2].col = normal[2];
		push.handedness = (glm::determinant(transformLinear) < 0.0f ? -1.0f : 1.0f);
		cmd.pushConstants(RES_IMPL(Shader, Vulkan, *material->GetShader()).layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(glm::mat4), &push);

		//Draw mesh
		cmd.drawIndexed(vkMesh.indices.size() * 3, 1, 0, 0, 0);
	}
}