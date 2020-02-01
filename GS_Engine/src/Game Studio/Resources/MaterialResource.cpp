#include "MaterialResource.h"

#include <fstream>
#include "Debug/Logger.h"
#include "ResourceManager.h"
#include "TextureResource.h"

InStream& operator>>(InStream& _I, MaterialResource::MaterialData& _MD)
{
	_I >> _MD.ResourceName >> _MD.VertexShaderCode >> _MD.FragmentShaderCode;
	return _I;
}

OutStream& operator<<(OutStream& _O, MaterialResource::MaterialData& _MD)
{
	_O << _MD.ResourceName << _MD.VertexShaderCode << _MD.FragmentShaderCode;
	return _O;
}

bool MaterialResource::loadResource(const LoadResourceData& LRD_)
{
	std::ifstream Input(LRD_.FullPath.c_str(), std::ios::in); //Open file as binary

	if (Input.is_open()) //If file is valid
	{
		Input.seekg(0, std::ios::end); //Search for end
		uint64 FileLength = Input.tellg(); //Get file length
		Input.seekg(0, std::ios::beg); //Move file pointer back to beginning

		InStream in_archive(&Input);

		in_archive >> data.ResourceName;

		in_archive >> data.VertexShaderCode;
		in_archive >> data.FragmentShaderCode;
	}
	else
	{
		Input.close();
		return false;
	}

	Input.close();

	return true;
}

void MaterialResource::makeFromData(ResourceData& resourceData)
{
	data = reinterpret_cast<MaterialData&>(resourceData);
}

void MaterialResource::loadFallbackResource(const FString& _Path)
{
}
