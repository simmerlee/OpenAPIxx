#ifndef _OPENAPI_THREAD_H_
#define _OPENAPI_THREAD_H_

#include <string>

namespace OpenAPI
{

class Thread
{
public:
	Thread();
	explicit Thread(const std::string& threadName);
	virtual ~Thread();
	int start();
	int join();
	int setThreadName(const std::string& threadName);
	int getThreadName(std::string& threadName);
	int getThreadId(unsigned int& threadId);
	static unsigned int GetCurrentThreadId();
protected:
	virtual void run();
private:
	class ThreadPrivate;
	friend ThreadPrivate;
	ThreadPrivate* m_p;
};

}// OpenAPI

typedef OpenAPI::Thread OAThread;

#endif//_OPENAPI_THREAD_H_

