#pragma once

#ifdef _WIN32
#ifdef CACAO_BUILD
#define CACAO_API __declspec(dllexport)
#else
#define CACAO_API __declspec(dllimport)
#endif
#else
#define CACAO_API
#endif