#include "FileLogger.h"

FileLogger::FileLogger()
{
	logfile_stream = new std::ofstream("./debug.log", std::ios::app);
	*logfile_stream << "\n==========================\n " << GetTimestamp() << "\n==========================\n" << std::endl;
}

FileLogger::~FileLogger()
{
	logfile_stream->close();
	delete logfile_stream;
}

std::string FileLogger::GetTimestamp()
{
	auto time = std::chrono::system_clock::now();
	std::time_t timestamp = std::chrono::system_clock::to_time_t(time);

	char timestring_buffer[30]{};

	ctime_s(timestring_buffer, 30, &timestamp);

	std::string timestring(timestring_buffer);

	timestring.erase(std::remove(timestring.begin(), timestring.end(), '\n'), timestring.end());

	return timestring;
}