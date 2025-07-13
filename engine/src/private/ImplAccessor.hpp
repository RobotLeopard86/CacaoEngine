#pragma once

#include "Cacao/Window.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/Mesh.hpp"
#include "Cacao/Tex2D.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Model.hpp"

#define IMPL(tp, ...) ImplAccessor::Get().Get##tp(__VA_ARGS__)
#define WIN_IMPL(tp) static_cast<tp##WindowImpl&>(ImplAccessor::Get().GetWindow())

#define IA_MKGETTER(tp)         \
	tp::Impl& Get##tp(tp& _o) { \
		return *_o.impl;        \
	}
#define IA_MKGETTER_SINGLE(tp)  \
	tp::Impl& Get##tp() {       \
		return *tp::Get().impl; \
	}

namespace Cacao {
	class ImplAccessor {
	  public:
		static ImplAccessor& Get();

		ImplAccessor(const ImplAccessor&) = delete;
		ImplAccessor(ImplAccessor&&) = delete;
		ImplAccessor& operator=(const ImplAccessor&) = delete;
		ImplAccessor& operator=(ImplAccessor&&) = delete;

		//Singletons
		IA_MKGETTER_SINGLE(Window)
		IA_MKGETTER_SINGLE(ResourceManager)
		IA_MKGETTER_SINGLE(PAL)

		//Resources
		IA_MKGETTER(Sound)
		IA_MKGETTER(Mesh)
		IA_MKGETTER(Tex2D)
		IA_MKGETTER(Cubemap)
		IA_MKGETTER(Model)

	  private:
		ImplAccessor();
	};
}