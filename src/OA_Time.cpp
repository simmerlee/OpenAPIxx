#include "OA_Time.h"
#include "OA_ErrorNumber.h"
#include "OA_Platform.h"
#include <time.h>

#ifdef OA_PLT_WINDOWS
#include <sys/types.h>
#include <sys/timeb.h>
#include <Windows.h>    // for GetTickCount64()
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
#include <sys/time.h>
#include <unistd.h>
#endif//OA_PLT_UNIX_FAMILY
#ifdef OA_PLT_MACOSX
#include <sys/types.h>
#include <sys/sysctl.h>
#endif//OA_PLT_MACOSX

#define TIME_TO_STRING_BUF_SIZE			256      // byte
#define OA_TIME_SLEEP_MAX_VALUE			4294967
#define OA_TIME_SLEEPMS_MAX_VALUE		4294967

uint64_t OpenAPIxx::Time::CurrentLocalTime()
{
    unsigned long long ms;
#ifdef OA_PLT_WINDOWS
    __timeb64 temp;
    ::_ftime64_s(&temp);
    
	ms = (uint64_t)(temp.time) * 1000LL + (uint64_t)(temp.millitm);
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    struct timeval tv;
	::gettimeofday(&tv, NULL);
	ms = tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
#endif//OA_PLT_UNIX_FAMILY

	return ms;
}

uint64_t OpenAPIxx::Time::TickCount64()
{
#ifdef OA_PLT_WINDOWS
    return GetTickCount64();
#endif//OA_PLT_WINDOWS
#if OA_PLT_LINUX || OA_PLT_BSD
	struct timespec ts;
	::clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);
#endif//OA_PLT_LINUX || OA_PLT_BSD
#ifdef OA_PLT_MACOSX
    int mib[2];
    struct timeval boottime_tv;
    struct timeval curtime_tv;
    size_t len = sizeof(boottime_tv);
    uint64_t boottime_ts;
    uint64_t curtime_ts;
    mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;
    ::sysctl(mib, 2, &boottime_tv, &len, NULL, 0);
    ::gettimeofday(&curtime_tv, NULL);
    boottime_ts = (uint64_t)(boottime_tv.tv_sec) * (uint64_t)1000
                + (uint64_t)(boottime_tv.tv_usec) / (uint64_t)1000;
    curtime_ts = (uint64_t)(curtime_tv.tv_sec) * (uint64_t)1000
                + (uint64_t)(curtime_tv.tv_usec) / (uint64_t)1000;
    return curtime_ts - boottime_ts;
#endif//OA_PLT_MACOXS
}

int32_t OpenAPIxx::Time::Sleep(uint32_t second)
{
	if (second > OA_TIME_SLEEP_MAX_VALUE) {
		return OA_ERR_OUT_OF_RANGE;
	}
#ifdef OA_PLT_WINDOWS
    ::Sleep(second * 1000);
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	::sleep(second);
#endif//OA_PLT_UNIX_FAMILY

	return OA_ERR_NO_ERROR;
}

int32_t OpenAPIxx::Time::SleepMs(uint32_t millisecond)
{
	if (millisecond > OA_TIME_SLEEP_MAX_VALUE) {
		return OA_ERR_OUT_OF_RANGE;
	}
#ifdef OA_PLT_WINDOWS
    ::Sleep(millisecond);
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	::usleep(millisecond * 1000);
#endif//OA_PLT_UNIX_FAMILY

	return OA_ERR_NO_ERROR;
}

int32_t OpenAPIxx::Time::TimestampToString(uint64_t timestamp,
	const std::string& format, std::string& str) {
	char buf[TIME_TO_STRING_BUF_SIZE];
	time_t second = (time_t)(timestamp / (uint64_t)1000);

    // return value of strftime(char* str, size_t count, 
    //                const char* format, const struct tm* time):
    // The number of bytes written into the character array pointed to 
    // by str not including the terminating '\0' on success.

#ifdef OA_PLT_WINDOWS
	tm tm_val;
	localtime_s(&tm_val, &second);
	if (strftime(buf, TIME_TO_STRING_BUF_SIZE, format.c_str(), &tm_val) == 0) {
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	tm* tm_val;
    tm_val = ::localtime(&second);
	if (::strftime(buf, TIME_TO_STRING_BUF_SIZE, format.c_str(), tm_val) == 0) {
#endif//OA_PLT_UNIX_FAMILY
		return OA_ERR_OPERATION_FAILED;
	}

    str.assign(buf);
	return OA_ERR_NO_ERROR;
}
