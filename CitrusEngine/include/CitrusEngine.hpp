#pragma once

//This file serves as an include for client applications and is not used in the Citrus Engine internal codebase

#include "Core/Entrypoint.hpp"
#include "Core/CitrusClient.hpp"
#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Events/EventSystem.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Graphics/Textures/TextureCube.hpp"
#include "Graphics/Skybox.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/Transform.hpp"
#include "Graphics/Cameras/Camera.hpp"
#include "Graphics/Cameras/OrthographicCamera.hpp"
#include "Graphics/Cameras/PerspectiveCamera.hpp"
#include "Models/Model.hpp"
#include "Utilities/Utilities.hpp"
#include "Utilities/Input.hpp"
#include "Utilities/StateManager.hpp"

//Include ImGui so that it can be used to draw UI
#include "imgui/imgui.h"
//Include GLM so that it's types can be used when interfacing with the engine
#include "glm/glm.hpp"