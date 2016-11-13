#ifndef _OPENAPI_TIME_H_
#define _OPENAPI_TIME_H_

#include <string>

struct timeval;

namespace OpenAPIxx
{

class Time
{
public:
    Time();
    Time(unsigned long long millisecond);
    unsigned int toSecond() const;
    unsigned long long toMillisecond() const;
    int toString(const std::string& format, std::string& output) const;
	int toTimeval(timeval* tv) const;

    static Time CurrentLocalTime();
    static unsigned long long TickCount64();
    static void Sleep(unsigned second);
    static void Msleep(unsigned millisecond);
	static int SetTimeval(unsigned millisecond, timeval* tv);
private:
    unsigned long long m_ms;
};

}// namespace

typedef OpenAPIxx::Time OATime;

#endif//_OPENAPI_TIME_H_
