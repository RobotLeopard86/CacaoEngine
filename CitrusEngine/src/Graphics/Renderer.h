#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Shader.h"

#include "glm/vec3.hpp"

namespace CitrusEngine {
    
    class Renderer {
    public:
        virtual ~Renderer() = 0;

        //Creates a renderer
        static void Create();
        //Shuts down a renderer
        static void Shutdown();

        //Sets clear color (takes 8-bit integer vector (0-255 for red, green, and blue))
        static void SetClearColor(glm::i8vec3 color);
        //Clears color and depth buffers
        static void Clear();
        //Renders some geometry
        static void RenderGeometry(Mesh mesh, Transform transform, Shader shader);
    protected:
        //Implementation of SetClearColor
        virtual void SetClearColor_Impl(glm::i8vec3 color) {}
        //Implementation of Clear
        virtual void Clear_Impl() = 0;
        //Implementation of RenderGeometry
        virtual void RenderGeometry_Impl(Mesh mesh, Transform transform, Shader shader) {}

        //Creates renderer for the native platform
        static Renderer* CreateNativeRenderer();
    private:
        static Renderer* instance;
    };
}