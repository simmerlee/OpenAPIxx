#include "OA_Thread.h"
#include "OA_ErrorNumber.h"

#include <pthread.h>
#include <linux/version.h>
#include <syscall.h>
#include <unistd.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)
#include <sys/prctl.h>
#endif

using namespace std;

class OpenAPIxx::Thread::ThreadPrivate
{
public:
	ThreadPrivate():threadId(0){}
	explicit ThreadPrivate(const std::string& _threadName) :
		threadName(_threadName),
		threadId(0){}
	static void* threadProc(void* arg)
	{
		Thread *thread = reinterpret_cast<Thread*>(arg);
		thread->m_p->threadId = Thread::GetCurrentThreadId();
		thread->run();
		return NULL;
	}

	pthread_t threadHandle;
	string threadName;
	unsigned int threadId;
};

OpenAPIxx::Thread::Thread():
	m_p(NULL)
{
	m_p = new ThreadPrivate();
}

OpenAPIxx::Thread::Thread(const std::string& threadName):
	m_p(NULL)
{
	m_p = new ThreadPrivate(threadName);
}

OpenAPIxx::Thread::~Thread()
{
	if(m_p != NULL)
	{
		delete m_p;
	}
}

int OpenAPIxx::Thread::start()
{
	if(pthread_create(&(m_p->threadHandle), NULL, ThreadPrivate::threadProc, this) != 0)
		return OA_ERR_SYSTEM_CALL_FAILED;
	return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Thread::join()
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;
	if (pthread_join(m_p->threadHandle, NULL) == 0)
		return OA_ERR_NO_ERROR;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPIxx::Thread::setThreadName(const std::string& threadName)
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;
#	if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)
	// prctl() return -1 when error (man-page)
	if(prctl(PR_SET_NAME, threadName.c_str()) == -1)
		return OA_ERR_SYSTEM_CALL_FAILED;
	else
		return OA_ERR_NO_ERROR;
#	endif// linux version new enough
	return OA_ERR_NOT_SUPPORT;
}

int OpenAPIxx::Thread::getThreadName(std::string& threadName)
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;

	threadName = m_p->threadName;
	return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Thread::getThreadId(unsigned int& threadId)
{
	if(m_p == NULL)
	{
		threadId = 0;
		return OA_ERR_OPERATION_FAILED;
	}
	threadId = m_p->threadId;
	return OA_ERR_NO_ERROR;
}

void OpenAPIxx::Thread::run() {}

unsigned int OpenAPIxx::Thread::GetCurrentThreadId()
{
	return (unsigned int)syscall(__NR_gettid);
}

