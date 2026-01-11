#pragma once

#include "Cacao/GPU.hpp"
#include <type_traits>

template<typename D>
	requires std::is_base_of_v<Cacao::CommandBuffer, D>
inline std::unique_ptr<D> CBCast(std::unique_ptr<Cacao::CommandBuffer>&& cb) {
	return std::unique_ptr<D>(static_cast<D*>(cb.release()));
}