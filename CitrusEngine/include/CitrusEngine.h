#pragma once

//This file serves as an include for client applications and is not used in the Citrus Engine internal codebase

#include "Core/Entrypoint.h"
#include "Core/CitrusClient.h"
#include "Core/Log.h"
#include "Core/Assert.h"
#include "Events/EventSystem.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Transform.h"
#include "Graphics/Cameras/Camera.h"
#include "Graphics/Cameras/OrthographicCamera.h"
#include "Graphics/Cameras/PerspectiveCamera.h"
#include "Models/Model.h"
#include "Utilities/Utilities.h"
#include "Utilities/Input.h"
#include "Utilities/StateManager.h"

//Include ImGui so that it can be used
#include "imgui/imgui.h"