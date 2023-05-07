#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Shader.h"

#include "Cameras/Camera.h"

#include "glm/vec3.hpp"

namespace CitrusEngine {
    
    class Renderer {
    public:
        virtual ~Renderer() {};

        //Creates a renderer
        static void Create();
        //Shuts down a renderer
        static void Shutdown();

        //Sets clear color (takes 8-bit unsigned integer vector (0-255 for red, green, and blue))
        static void SetClearColor(glm::u8vec3 color);
        //Clears color and depth buffers
        static void Clear();
        //Renders some geometry
        static void RenderGeometry(Mesh* mesh, Transform* transform, Shader* shader);
        //Set viewport width and height
        static void ResizeViewport(int width, int height);
        //Initialize rendering backend
        static void InitBackend();
        //Set the camera to use for rendering
        static void SetCamera(Camera* cam);
    protected:
        //Implementation of SetClearColor
        virtual void SetClearColor_Impl(glm::u8vec3 color) = 0;
        //Implementation of Clear
        virtual void Clear_Impl() = 0;
        //Implementation of RenderGeometry
        virtual void RenderGeometry_Impl(Mesh* mesh, Transform* transform, Shader* shader) = 0;
        //Implementation of ResizeViewport
        virtual void ResizeViewport_Impl(int width, int height) = 0;
        //Implementation of InitBackend
        virtual void InitBackend_Impl() = 0;
        //Implementation of SetCamera
        virtual void SetCamera_Impl(Camera* cam) = 0;

        //Creates renderer for the native platform
        static Renderer* CreateNativeRenderer();
    private:
        static Renderer* instance;
    };
}