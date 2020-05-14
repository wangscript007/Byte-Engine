#include "Logger.h"

#include "Byte Engine/Application/Application.h"
#include "Byte Engine/Application/Clock.h"

#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GTSL/Array.hpp>
#include <GTSL\StaticString.hpp>

using namespace BE;

BE::PersistentAllocatorReference allocator_reference("Clock", true); //TODO get rid of global once vector is fixed

Logger::Logger(const LoggerCreateInfo& loggerCreateInfo) : logFile(), fileBuffer(defaultBufferLength * 2/*use single buffer as two buffers for when one half is being written to disk*/, &allocator_reference)
{
	GTSL::StaticString<1024> path(loggerCreateInfo.AbsolutePathToLogDirectory);
	path += "/log.txt";
	fileBuffer.Resize(fileBuffer.GetCapacity());
	logFile.OpenFile(path, GTSL::File::OpenFileMode::WRITE);
	GTSL::Memory::SetZero(defaultBufferLength * 2, fileBuffer.GetData());
}

void Logger::Shutdown() const
{
	uint64 bytes_written{ 0 };
	logMutex.Lock();
	logFile.WriteToFile(GTSL::Ranger<byte>(reinterpret_cast<byte*>(fileBuffer.begin()), reinterpret_cast<byte*>(fileBuffer.begin() + fileBuffer.GetLength())), bytes_written);
	logMutex.Unlock();
	logFile.CloseFile();
}

void Logger::log(const VerbosityLevel verbosityLevel, const GTSL::Ranger<GTSL::UTF8>& text) const
{
	const auto day_of_month = Clock::GetDayOfMonth(); const auto month = Clock::GetMonth(); const auto year = Clock::GetYear(); const auto time = Clock::GetTime();

	char string[logMaxLength]{ 0 };
	
	uint32 string_remaining_length{ logMaxLength };
	uint32 write_start{ lastWriteToDiskPos + bytesWrittenSinceLastWriteToDisk };
	uint32 written_bytes{ 0 };
	
	// Check if should dump logs to file if no more space is available
	if (write_start >= bytesToDumpOn)
	{
		if(write_start >= defaultBufferLength * 2)
		{
			currentStringIndex = 0;
		}
		
		uint64 bytes_written{ 0 };
		//TODO dispatch as a job
		logMutex.Lock();
		logFile.WriteToFile(GTSL::Ranger<byte>((byte*)fileBuffer.begin() + lastWriteToDiskPos, (byte*)fileBuffer.begin() + lastWriteToDiskPos + bytesWrittenSinceLastWriteToDisk), bytes_written);
		logMutex.Unlock();
		
		lastWriteToDiskPos += bytesWrittenSinceLastWriteToDisk;
		bytesWrittenSinceLastWriteToDisk = 0;
		write_start = lastWriteToDiskPos;
	}

	auto write_loc = [&]() {return written_bytes; };
	auto write_ptr = [&]() { return string + write_loc(); };

	const uint32 date_length = snprintf(write_ptr(), string_remaining_length, "[Date: %02d/%02hhu/%02d]", day_of_month, month, year);
	string_remaining_length -= date_length;
	written_bytes += date_length;

	const uint32 time_length = snprintf(write_ptr(), string_remaining_length, "[Time: %02d:%02d:%02d]", time.Hour, time.Minute, time.Second);
	string_remaining_length -= time_length;
	written_bytes += time_length;

	const uint32 text_length = text.ElementCount(); //TODO: ASSERT LENGTH
	string_remaining_length -= text_length;
	GTSL::Memory::MemCopy(text_length, text.Data(), write_ptr());
	written_bytes += text_length;

	written_bytes += 1; //null terminator
	
	string[write_loc() - 1] = '\n';
	string[write_loc()] = '\0';

	logMutex.Lock();
	GTSL::Memory::MemCopy(written_bytes, string, fileBuffer.GetData() + write_start);
	logMutex.Unlock();
	
	if(verbosityLevel >= minLogLevel)
	{
		SetTextColorOnLogLevel(verbosityLevel);
		printf(string + date_length);
	}

	++currentStringIndex;
	bytesWrittenSinceLastWriteToDisk += written_bytes;
}

void Logger::PrintObjectLog(const Object* obj, const VerbosityLevel level, const char* text, ...) const
{
	GTSL::StaticString<1024> t;
	t += obj->GetName(); t += ": "; t += text;
	log(level, t);
}

void Logger::SetTextColorOnLogLevel(const VerbosityLevel level) const
{
	switch (level)
	{
	default: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); break;

	case VerbosityLevel::MESSAGE: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); break;

	case VerbosityLevel::SUCCESS: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;

	case VerbosityLevel::WARNING: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN); break;

	case VerbosityLevel::FATAL: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY); break;
	}
}

Logger::~Logger()
{
}

void Logger::PrintBasicLog(const VerbosityLevel level, const char* text, ...) const
{
	GTSL::StaticString<1024> t;
	t += text;
	log(level, t);
}
