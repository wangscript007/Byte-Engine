#include "MaterialResource.h"

#include <fstream>
#include <string>

std::istream& operator>>(std::istream& _I, MaterialResource::MaterialData& _MD)
{
	_I >> _MD.VertexShaderCode >> _MD.FragmentShaderCode;
	return _I;
}

std::ostream& operator<<(std::ostream& _O, MaterialResource::MaterialData& _MD)
{
	_O << _MD.VertexShaderCode << _MD.FragmentShaderCode;
	return _O;
}


bool MaterialResource::LoadResource(const FString& _Path)
{
	std::ifstream Input(_Path.c_str(), std::ios::in | std::ios::binary);	//Open file as binary

	if(Input.is_open())	//If file is valid
	{
		Input.seekg(0, std::ios::end);	//Search for end
		uint64 FileLength = Input.tellg();		//Get file length
		Input.seekg(0, std::ios::beg);	//Move file pointer back to beginning

		Data = new MaterialData;	//Intantiate resource data

		Input >> *SCAST(MaterialData*, Data);

		//size_t HeaderCount = 0;
		//Input.read(&reinterpret_cast<char&>(HeaderCount), sizeof(ResourceHeaderType));	//Get header count from the first element in the file since it's supposed to be a header count variable of type ResourceHeaderType(uint64) as per the engine spec.
		//
		//ResourceElementDescriptor CurrentFileElementHeader;
		//Input.read(&reinterpret_cast<char&>(CurrentFileElementHeader), sizeof(CurrentFileElementHeader));
		//Input.read(&reinterpret_cast<char&>(Data->GetResourceName() = new char[CurrentFileElementHeader.Bytes]), CurrentFileElementHeader.Bytes);
		//
		//for(size_t i = 0; i < HeaderCount; ++i)		//For every header in the file
		//{
		//	Input.read(&reinterpret_cast<char&>(CurrentFileElementHeader), sizeof(ResourceSegmentType));	//Copy next section size from section header
		//	Input.read(reinterpret_cast<char*>(Data->WriteTo(i, CurrentFileElementHeader.Bytes)), CurrentFileElementHeader.Bytes);	//Copy section data from file to resource data
		//}

		//Input.read(&reinterpret_cast<char&>(CurrentFileElementHeader), sizeof(CurrentFileElementHeader));
		//Input.read(SCAST(MaterialData*, Data)->VertexShaderCode = new char[CurrentFileElementHeader.Bytes], CurrentFileElementHeader.Bytes);
		//Input.read(&reinterpret_cast<char&>(CurrentFileElementHeader), sizeof(CurrentFileElementHeader));
		//Input.read(SCAST(MaterialData*, Data)->FragmentShaderCode = new char[CurrentFileElementHeader.Bytes], CurrentFileElementHeader.Bytes);
		//Input.read(&reinterpret_cast<char&>(CurrentFileElementHeader), sizeof(CurrentFileElementHeader));
		//Input.read(&reinterpret_cast<char&>(SCAST(MaterialData*, Data)->ShaderDynamicParameters), CurrentFileElementHeader.Bytes);
	}
	else
	{
		Input.close();
		return false;
	}

	Input.close();

	return true;
}

void MaterialResource::LoadFallbackResource(const FString& _Path)
{
}