#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Shader.h"

#include "Cameras/Camera.h"

#include "glm/vec3.hpp"

namespace CitrusEngine {
    //Renderer singleton
    class Renderer {
    public:
        virtual ~Renderer() {}

        //Sets clear color (takes 8-bit unsigned integer vector (0-255 for red, green, and blue))
        virtual void SetClearColor(glm::u8vec3 color) = 0;
        //Clears color and depth buffers
        virtual void Clear() = 0;
        //Renders some geometry
        virtual void RenderGeometry(Mesh* mesh, Transform* transform, Shader* shader) = 0;
        //Set viewport width and height
        virtual void ResizeViewport(int width, int height) = 0;
        //Initialize rendering backend
        virtual void InitBackend() = 0;
        //Set the camera to use for rendering
        virtual void SetCamera(Camera* cam) = 0;

        //Get the current instance or create one if it doesn't exist
        static Renderer* GetInstance();
    protected:
        //Creates renderer for the native platform (implemented by subclasses)
        static Renderer* CreateNativeRenderer();

        //Protected constructor so only subclasses can call it
        Renderer() {}
    private:
        static Renderer* instance;
        static bool instanceExists;
    };
}