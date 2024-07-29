#pragma once

//Just a quick utility header to include either the OpenGL or OpenGL (ES) headers, dependening on which one is necessary
#ifdef ES
#include "glad/gles2.h"
#else
#include "glad/gl.h"
#endif