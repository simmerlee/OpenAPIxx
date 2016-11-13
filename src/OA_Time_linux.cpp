#include "OA_ErrorNumber.h"
#include "OA_Time.h"
#include <ctime>
#include <sys/time.h>
#include <unistd.h>

#define TIME_TO_STRING_BUFFER_EXTRAL_RESERVE_SIZE   64      // byte

OpenAPIxx::Time::Time():m_ms(0){}

OpenAPIxx::Time::Time(unsigned long long millisecond):m_ms(millisecond){}

unsigned int OpenAPIxx::Time::toSecond() const
{
    return (unsigned int)(m_ms / 1000LL);
}

unsigned long long OpenAPIxx::Time::toMillisecond() const
{
    return m_ms;
}

int OpenAPIxx::Time::toString(const std::string& format, 
                            std::string& output) const
{
    unsigned bufSize = format.size() 
                     + TIME_TO_STRING_BUFFER_EXTRAL_RESERVE_SIZE;
    char* buf = new char[bufSize];
    int ret = OA_ERR_OPERATION_FAILED;

    time_t second = (time_t)(m_ms / 1000LL);
    tm* time;
    time = ::localtime(&second);
    if(::strftime(buf, bufSize, format.c_str(), time) != 0)
    {
        ret = OA_ERR_NO_ERROR;
        output.assign(buf);
    }
    delete buf;

    return ret;
}

int OpenAPIxx::Time::toTimeval(timeval* tv) const
{
	if (tv == NULL)
		return OA_ERR_ILLEGAL_ARG;
	tv->tv_sec = (unsigned)(m_ms / 1000LL);
	tv->tv_usec = (unsigned)(m_ms % 1000LL * 1000LL);

	return OA_ERR_NO_ERROR;
}

OpenAPIxx::Time OpenAPIxx::Time::CurrentLocalTime()
{
    timeval tv;
    ::gettimeofday(&tv, NULL);
    unsigned long long ms = tv.tv_sec * 1000LL
                          + tv.tv_usec / 1000LL;

    return Time(ms);
}

unsigned long long OpenAPIxx::Time::TickCount64()
{
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);
}

void OpenAPIxx::Time::Sleep(unsigned second)
{
    ::sleep(second);
}

void OpenAPIxx::Time::Msleep(unsigned millisecond)
{
    ::usleep(millisecond * 1000);
}

int OpenAPIxx::Time::SetTimeval(unsigned millisecond, timeval* tv)
{
	if (tv == NULL)
		return OA_ERR_ILLEGAL_ARG;
	tv->tv_sec = (unsigned)(millisecond / 1000LL);
	tv->tv_usec = (unsigned)(millisecond % 1000LL * 1000LL);

	return OA_ERR_NO_ERROR;
}

