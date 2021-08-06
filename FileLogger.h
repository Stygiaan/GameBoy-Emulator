#pragma once

#include <fstream>
#include <chrono>
#include <time.h>
#include <string>
#include <memory>
#include <cstdarg>

class FileLogger
{
public:
	FileLogger();
	~FileLogger();

	template<typename T>
	void Log(T&& msg)
	{
		*logfile_stream << msg << "\n";
		logfile_stream->flush();
	}
	template<typename Head, typename... Tail>
	void Log(Head&& head, Tail&&... tail) {
		*logfile_stream << head;
		logfile_stream->flush();

		Log(std::forward<Tail>(tail)...);
	}
	//template <typename... Args>
	//void Log(Args... args)
	//{
	//	std::va_list args;

	//	*logfile_stream << "[" << GetTimestamp() << "]: " << "\n";

	//	/*for (msg : args)
	//	{

	//	}*/
	//}
	//void Log(std::string&& message);
	std::string GetTimestamp();  

private:
	std::ofstream* logfile_stream{};
};

