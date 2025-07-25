#pragma once

#include "Cacao/Tex2D.hpp"

#include <optional>

namespace Cacao {
	class Tex2D::Impl {
	  public:
		virtual std::optional<std::shared_future<void>> Realize() = 0;
		virtual void DropRealized() = 0;

		/*
		Unlike the other functions here, this is not useful for data state.
		Some APIs (like OpenGL) operate on a single thread. Therefore, most operations are async by default, since they have to be run on the context thread and this may not be immediate.
		By contrast, other APIs (like Vulkan) do not care about threading. Therefore, most operations are sync by default, since they can be run anywhere.

		What this tells you is whether the synchronous realization function should be wait on the asynchronous one, or whether the asynchronous one should put the synchronous one in the thread pool.
		*/
		virtual bool DoWaitAsyncForSync() const = 0;

		libcacaoimage::Image img;

		virtual ~Impl() = default;
	};
}