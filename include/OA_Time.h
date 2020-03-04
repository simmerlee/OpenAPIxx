#ifndef _OPENAPI_TIME_H_
#define _OPENAPI_TIME_H_

#include <string>
#include <stdint.h>

namespace OpenAPIxx
{

class Time
{
public:
	// return current local time in ms
	// which is the time scince 1970-1-1 0:0:0
	static uint64_t CurrentLocalTime();

	// return the time since the computer powered on in ms
    static uint64_t TickCount64();

	// Sleep specific second
	// NOTICE: second MUST less or equal than 4294967
    static int32_t Sleep(uint32_t second);

	// Sleep specific millisecond
	// NOTICE: millisecond MUST less or equal than 4294967
	static int32_t SleepMs(uint32_t millisecond);

	// Convert unix timestamp in ms to string
	// which is similar as strftime() in standard C lib
    // NOTICE: the length of result string should less than 256 byte
	static int32_t TimestampToString(uint64_t timestamp,
		const std::string& format, std::string& str);
};

}// namespace

typedef OpenAPIxx::Time OATime;

#endif//_OPENAPI_TIME_H_
