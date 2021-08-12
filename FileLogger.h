#pragma once

#include <fstream>
#include <chrono>
#include <time.h>
#include <string>
#include <memory>
#include <cstdarg>
#include <map>

#define LOG_INFO 0
#define LOG_WARNING 1
#define LOG_ERROR 2

class FileLogger
{
public:
	FileLogger();
	~FileLogger();

	std::string GetTimestamp();

	template<typename T>
	void Log(int type, T&& msg)
	{
		*logfile_stream << logtype[type] << msg << std::endl;
	}
	template<typename Head, typename... Tail>
	void Log(int type, Head&& head, Tail&&... tail) {
		*logfile_stream << logtype[type] << head;

		Log(std::forward<Tail>(tail)...);
	}

private:
	template<typename T>
	void Log(T&& msg)
	{
		*logfile_stream << msg << std::endl;
	}

	std::ofstream* logfile_stream{};
	std::map<int, std::string> logtype = { {0, "[Info]    "}, {1, "[Warning] "}, {2, "[Error]   "} };
};

