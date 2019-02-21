#include "String.hpp"

String::String() : Array(10)
{
}

String::String(const char * In) : Array(const_cast<char *>(In), StringLength(In))
{
}

String::String(const char * In, const size_t Length) : Array(const_cast<char *>(In), Length)
{
}

String::~String()
{
}

String & String::operator=(const char * In)
{
	Array.overlay(0, const_cast<char *>(In), StringLength(In));

	return *this;
}

String & String::operator=(const String & Other)
{
	Array = Other.Array;

	return *this;
}

const char * String::c_str()
{
	return Array.data();
}

void String::Append(const char * In)
{
	Array.push_back(const_cast<char *>(In), StringLength(In));

	return;
}

void String::Insert(const char * In, size_t Index)
{
	Array.insert(Index, const_cast<char *>(In), StringLength(In));

	return;
}

size_t String::StringLength(const char* In)
{
	size_t Length = 0;

	while(In[Length] != '\0')
	{
		Length++;
	}

	//We return Length + 1 to take into account for the null terminator character.
	return Length + 1;
}
