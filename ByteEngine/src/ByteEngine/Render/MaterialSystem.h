#pragma once

#include <GAL/RenderCore.h>
#include <GTSL/Array.hpp>
#include <GTSL/Buffer.h>
#include <GTSL/Vector.hpp>

#include "RenderTypes.h"
#include "ByteEngine/Game/System.h"
#include "ByteEngine/Resources/MaterialResourceManager.h"

struct TaskInfo;
class RenderSystem;

class MaterialSystem : public System
{
public:
	MaterialSystem() : System("MaterialSystem"), materialNames(16, GetPersistentAllocator())
	{}

	void Initialize(const InitializeInfo& initializeInfo) override;
	void Shutdown(const ShutdownInfo& shutdownInfo) override;
	
	ComponentReference CreateMaterial(Id name);

	void SetMaterialParameter(const ComponentReference material, GAL::ShaderDataType type, Id32 parameterName, void* data)
	{
		uint32 parameter = 0;
		for (auto e : shaderParameters[material].ParameterNames) { if (e == parameterName) break; ++parameter; }

		byte* FILL = 0;
		GTSL::MemCopy(GAL::ShaderDataTypesSize(type), data, FILL + shaderParameters[material].ParameterOffset[parameter]);
	}

	void SetMaterialTexture(const ComponentReference material, Image* image)
	{
		
	}

	
private:
	struct ShaderParameters
	{
		GTSL::Array<Id32, 12> ParameterNames;

		GTSL::Array<uint32, 12> ParameterOffset;
	};
	
	Vector<Id> materialNames;
	Vector<ShaderParameters> shaderParameters;

	ComponentReference component = 0;

	struct MaterialLoadInfo
	{
		MaterialLoadInfo(RenderSystem* renderSystem, GTSL::Buffer&& buffer, uint32 instance) : RenderSystem(renderSystem), Buffer(MoveRef(buffer)), Instance(instance)
		{

		}

		RenderSystem* RenderSystem = nullptr;
		GTSL::Buffer Buffer;
		uint32 Instance = 0;
	};
	void onMaterialLoaded(TaskInfo taskInfo, MaterialResourceManager::OnMaterialLoadInfo onStaticMeshLoad);

	BindingsSetLayout bindingsSetLayout;
	GTSL::Vector<GraphicsPipeline, BE::PersistentAllocatorReference> pipelines;
	BindingsPool bindingsPool;
};
