#pragma once

#include "slang.h"
#include "slang-com-ptr.h"

using Slang::ComPtr;

#include <string>
#include <filesystem>
#include <utility>

class CacaoShaderCompiler {
  public:
	CacaoShaderCompiler();

	CacaoShaderCompiler(const CacaoShaderCompiler&) noexcept = delete;
	CacaoShaderCompiler& operator=(const CacaoShaderCompiler&) = delete;

	CacaoShaderCompiler(CacaoShaderCompiler&&) noexcept = delete;
	CacaoShaderCompiler& operator=(CacaoShaderCompiler&&) = delete;

	std::pair<bool, std::string> compile(const std::filesystem::path& in, const std::filesystem::path& out);

  private:
	ComPtr<slang::IGlobalSession> gSession;
};