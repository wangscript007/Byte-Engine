#include "Game.h"

#include "SandboxGameInstance.h"
#include "SandboxWorld.h"
#include "ByteEngine/Application/InputManager.h"
#include "ByteEngine/Resources/MaterialResourceManager.h"

#include "ByteEngine/Game/CameraComponentCollection.h"

#include <GTSL/Math/AxisAngle.h>

void Game::moveLeft(InputManager::ActionInputEvent data)
{
	moveDir.X = -data.Value;
}

void Game::moveForward(InputManager::ActionInputEvent data)
{
	moveDir.Z = data.Value;
}

void Game::moveBackwards(InputManager::ActionInputEvent data)
{
	moveDir.Z = -data.Value;
}

void Game::moveRight(InputManager::ActionInputEvent data)
{
	moveDir.X = data.Value;
}

void Game::Initialize()
{
	GameApplication::Initialize();

	BE_LOG_SUCCESS("Inited Game: ", GetApplicationName())
	
	gameInstance = GTSL::SmartPointer<GameInstance, BE::SystemAllocatorReference>::Create<SandboxGameInstance>(systemAllocatorReference);
	sandboxGameInstance = gameInstance;

	GTSL::Array<GTSL::Id64, 2> a({ GTSL::Id64("MouseMove") });
	inputManagerInstance->Register2DInputEvent("Move", a, GTSL::Delegate<void(InputManager::Vector2DInputEvent)>::Create<Game, &Game::move>(this));

	a.PopBack(); a.EmplaceBack("W_Key");
	inputManagerInstance->RegisterActionInputEvent("Move Forward", a, GTSL::Delegate<void(InputManager::ActionInputEvent)>::Create<Game, &Game::moveForward>(this));
	a.PopBack(); a.EmplaceBack("A_Key");
	inputManagerInstance->RegisterActionInputEvent("Move Left", a, GTSL::Delegate<void(InputManager::ActionInputEvent)>::Create<Game, &Game::moveLeft>(this));
	a.PopBack(); a.EmplaceBack("S_Key");
	inputManagerInstance->RegisterActionInputEvent("Move Backward", a, GTSL::Delegate<void(InputManager::ActionInputEvent)>::Create<Game, &Game::moveBackwards>(this));
	a.PopBack(); a.EmplaceBack("D_Key");
	inputManagerInstance->RegisterActionInputEvent("Move Right", a, GTSL::Delegate<void(InputManager::ActionInputEvent)>::Create<Game, &Game::moveRight>(this));
	

	GameInstance::CreateNewWorldInfo create_new_world_info;
	menuWorld = sandboxGameInstance->CreateNewWorld<MenuWorld>(create_new_world_info);

	/// <summary>
	/// Push bindings only for actual shader
	/// </summary>
	MaterialResourceManager::MaterialCreateInfo material_create_info;
	material_create_info.ShaderName = "BasicMaterial";
	GTSL::Array<GAL::ShaderDataType, 8> format{ GAL::ShaderDataType::FLOAT3, GAL::ShaderDataType::FLOAT3 };
	GTSL::Array<GTSL::Array<GAL::BindingType, 8>, 8> binding_sets(1);
	binding_sets[0].EmplaceBack(GAL::BindingType::UNIFORM_BUFFER_DYNAMIC);
	material_create_info.VertexFormat = format;
	material_create_info.ShaderTypes = GTSL::Array<GAL::ShaderType, 12>{ GAL::ShaderType::VERTEX_SHADER, GAL::ShaderType::FRAGMENT_SHADER };
	GTSL::Array<GTSL::Ranger<const GAL::BindingType>, 10> b_array;
	b_array.EmplaceBack(binding_sets[0]);
	material_create_info.BindingSets = b_array;
	GetResourceManager<MaterialResourceManager>("MaterialResourceManager")->CreateMaterial(material_create_info);
	
	//show loading screen//
	//load menu//
	//show menu
	//start game
}

void Game::PostInitialize()
{
	GameApplication::PostInitialize();

	camera = gameInstance->GetComponentCollection<CameraComponentCollection>("CameraComponentCollection")->AddCamera(GTSL::Vector3(0, 0, -250));
	
	auto* collection = gameInstance->GetComponentCollection<RenderStaticMeshCollection>("RenderStaticMeshCollection");
	auto component = collection->AddMesh();
	collection->SetMesh(component, "Box");
	collection->SetPosition(component, GTSL::Vector3(25, 50, 200));

	auto* static_mesh_renderer = gameInstance->GetSystem<StaticMeshRenderGroup>("StaticMeshRenderGroup");
	StaticMeshRenderGroup::AddStaticMeshInfo add_static_mesh_info;
	add_static_mesh_info.RenderSystem = gameInstance->GetSystem<RenderSystem>("RenderSystem");
	add_static_mesh_info.GameInstance = gameInstance;
	add_static_mesh_info.ComponentReference = component;
	add_static_mesh_info.RenderStaticMeshCollection = collection;
	add_static_mesh_info.StaticMeshResourceManager = GetResourceManager<StaticMeshResourceManager>("StaticMeshResourceManager");
	add_static_mesh_info.MaterialName = "BasicMaterial";
	add_static_mesh_info.MaterialResourceManager = GetResourceManager<MaterialResourceManager>("MaterialResourceManager");
	static_mesh_renderer->AddStaticMesh(add_static_mesh_info);

	//window.ShowMouse(false);
}

void Game::OnUpdate(const OnUpdateInfo& onUpdate)
{
	GameApplication::OnUpdate(onUpdate);

	gameInstance->GetComponentCollection<CameraComponentCollection>("CameraComponentCollection")->AddCameraPosition(camera, GTSL::Vector3(moveDir * 10));
}

void Game::Shutdown()
{
	GameApplication::Shutdown();
}

void Game::move(InputManager::Vector2DInputEvent data)
{
	posDelta += (data.Value - data.LastValue) * 10;
	
	auto rot = GTSL::Matrix4(GTSL::AxisAngle(0.f, 1.0f, 0.f, posDelta.X));
	rot *= GTSL::Matrix4(GTSL::AxisAngle(rot.GetXBasisVector(), -posDelta.Y));
	gameInstance->GetComponentCollection<CameraComponentCollection>("CameraComponentCollection")->SetCameraRotation(camera, rot);
}

Game::~Game()
{
}
