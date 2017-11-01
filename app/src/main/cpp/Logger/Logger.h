#pragma once
#include <string>
#include <fstream>

enum class LogType : uint8_t
{
	ERROR,
	WARNING,
	LOG,
};

class Logger
{
public:
	Logger();
	void LogString(const std::string & String, LogType Type = LogType::LOG);
	~Logger();

	void LogOnFile(bool LogEnable)
	{
		LogFileEnable = LogEnable;
	}

	static Logger & GetLogger();

	bool AssertOnError = true;

private:
	std::ofstream FileStream;
	bool LogFileEnable;
};


