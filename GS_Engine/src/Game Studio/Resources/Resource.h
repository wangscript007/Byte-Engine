#pragma once

#include "Core.h"

#include "Containers/FString.h"

//Base class representation of all types of resources that can be loaded into the engine.
GS_CLASS Resource
{
public:
	Resource() = default;

	Resource(const FString & Path) : FilePath(Path)
	{
	}

	virtual ~Resource() = default;


	//Returns the size of the data.
	[[nodiscard]] virtual size_t GetDataSize() const = 0;

	[[nodiscard]] const FString& GetPath() const { return FilePath; }

protected:
	void* Data = nullptr;

	//Resource identifier. Used to check if a resource has already been loaded.
	FString FilePath;

	virtual bool LoadResource() = 0;
	virtual void LoadFallbackResource() = 0;
};