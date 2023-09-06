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
        void SetClearColor(glm::u8vec3 color);
        //Clears color and depth buffers
        void Clear();
        //Renders some geometry
        void RenderGeometry(Mesh* mesh, Transform* transform, Shader* shader);
        //Initialize rendering backend
        void InitBackend();
        //Shutdown rendering backed
        void ShutdownBackend();
        //Set the camera to use for rendering
        void SetCamera(Camera* cam);

        //Get the current instance or create one if it doesn't exist
        static Renderer* GetInstance();
    protected:
        //Creates renderer for the native platform (implemented by subclasses)
        static Renderer* CreateNativeRenderer();

        //Protected constructor so only subclasses can call it
        Renderer();

        bool backendInitialized = false;
    private:
        static Renderer* instance;
        static bool instanceExists;

        glm::vec4 clearColor;
        Camera* activeCam;
    };
}