#ifndef _OPENAPI_THREAD_H_
#define _OPENAPI_THREAD_H_

#include <string>
#include <stdint.h>

namespace OpenAPIxx
{

// 在UNIX_FAMILY平台下
// GetLastError取得的是pthread_xxx的返回值

class Thread
{
public:
	Thread();
	explicit Thread(const std::string& threadName);
	virtual ~Thread();
	virtual void run();
	int32_t start();
    int32_t join();
    int32_t setThreadName(const std::string& threadName);
    int32_t getThreadName(std::string& threadName);
    int32_t getLastError();
private:
    void* m_p;
};

}// OpenAPIxx

typedef OpenAPIxx::Thread OAThread;

#endif//_OPENAPI_THREAD_H_

