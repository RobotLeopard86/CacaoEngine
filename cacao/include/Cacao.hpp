#pragma once

//Cacao Engine header file for client usage

#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "Events/EventSystem.hpp"
#include "3D/Mesh.hpp"
#include "3D/Model.hpp"
#include "3D/Transform.hpp"
#include "3D/Vertex.hpp"
#include "3D/Skybox.hpp"
#include "World/Entity.hpp"
#include "World/World.hpp"
#include "World/WorldManager.hpp"
#include "Scripts/Script.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Graphics/Rendering/MeshComponent.hpp"
#include "Graphics/Cameras/PerspectiveCamera.hpp"
#include "Utilities/Flushable.hpp"
#include "Utilities/Input.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/MultiFuture.hpp"
#include "Utilities/Asset.hpp"
#include "Utilities/AssetManager.hpp"
#include "Audio/Sound.hpp"
#include "Audio/AudioPlayer.hpp"
#include "UI/Font.hpp"
#include "UI/Screen.hpp"
#include "UI/Text.hpp"
#include "UI/Image.hpp"
#include "UI/UIElement.hpp"
#include "UI/UIView.hpp"

//Cacao namespace docs

/**
 * @brief The namespace where all @a Cacao @a Engine components live
 */
namespace Cacao {}