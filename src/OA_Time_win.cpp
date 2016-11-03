#include "OA_ErrorNumber.h"
#include "OA_Time.h"
#include <ctime>
#include <sys/types.h>
#include <sys/timeb.h>
#include <Windows.h>    // for GetTickCount64()

#define TIME_TO_STRING_BUFFER_EXTRAL_RESERVE_SIZE   64      // byte

OpenAPI::Time::Time():m_ms(0){}

OpenAPI::Time::Time(unsigned long long millisecond):m_ms(millisecond){}

unsigned int OpenAPI::Time::toSecond() const
{
    return (unsigned int)(m_ms / 1000LL);
}

unsigned long long OpenAPI::Time::toMillisecond() const
{
    return m_ms;
}

int OpenAPI::Time::toString(const std::string& format, 
                            std::string& output) const
{
    unsigned bufSize = format.size() 
                     + TIME_TO_STRING_BUFFER_EXTRAL_RESERVE_SIZE;
    char* buf = new char[bufSize];
    int ret = OA_ERR_OPERATION_FAILED;

    time_t second = (time_t)(m_ms / 1000LL);
    tm time;
    localtime_s(&time, &second);
    if(strftime(buf, bufSize, format.c_str(), &time) != 0)
    {
        ret = OA_ERR_NO_ERROR;
        output.assign(buf);
    }
    delete buf;
    return ret;
}

int OpenAPI::Time::toTimeval(timeval* tv) const
{
	if (tv == NULL)
		return OA_ERR_ILLEGAL_ARG;
	tv->tv_sec = (unsigned)(m_ms / 1000LL);
	tv->tv_usec = (unsigned)(m_ms % 1000LL * 1000LL);

	return OA_ERR_NO_ERROR;
}

OpenAPI::Time OpenAPI::Time::CurrentLocalTime()
{
    unsigned long long ms;
    __timeb64 temp;
    _ftime64_s(&temp);
    
    ms = (unsigned long long)(temp.time) * 1000LL
       + (unsigned long long)(temp.millitm);

    return Time(ms);
}

unsigned long long OpenAPI::Time::TickCount64()
{
    return GetTickCount64();
}

void OpenAPI::Time::Sleep(unsigned second)
{
    return ::Sleep(second * 1000);
}

void OpenAPI::Time::Msleep(unsigned millisecond)
{
    return ::Sleep(millisecond);
}

int OpenAPI::Time::SetTimeval(unsigned millisecond, timeval* tv)
{
	if (tv == NULL)
		return OA_ERR_ILLEGAL_ARG;
	tv->tv_sec = (unsigned)(millisecond / 1000LL);
	tv->tv_usec = (unsigned)(millisecond % 1000LL * 1000LL);

	return OA_ERR_NO_ERROR;
}
