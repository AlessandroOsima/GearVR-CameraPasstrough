#pragma once
#include <string>
#include <cassert>

#ifdef _DEBUG
#define FailWithMessage(string) {\
	Logger::GetLogger().LogString(string, LogType::ERROR); \
	assert(nullptr); \
} \

#define AssertWithMessage(condition, string) {\
	if (!(condition)) \
	{ \
		Logger::GetLogger().LogString(string, LogType::ERROR); \
		assert(nullptr); \
	} \
} \

//Just a simple wrapper over default assert
#define Assert(condition) {\
assert(condition); \
} \



#else

#define FailWithMessage(string) {\
} \

#define AssertWithMessage(condition, string) {\
} \

//Just a simple wrapper over default assert
#define Assert(condition) {\
assert(condition); \
} \

#endif