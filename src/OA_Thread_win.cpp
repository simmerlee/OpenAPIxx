#include "OA_Thread.h"
#include "OA_ErrorNumber.h"
#include <Windows.h>
#include <process.h>

using namespace std;

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(0x406D1388, 0, 
                       sizeof(info) / sizeof(DWORD), 
                       (DWORD*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

class OpenAPI::Thread::ThreadPrivate
{
public:
	ThreadPrivate() : threadHandle(NULL){}
	explicit ThreadPrivate(const std::string& _threadName) :
		threadHandle(NULL),
		threadName(_threadName) {}
	static unsigned int __stdcall threadProc(LPVOID arg)
	{
		Thread *thread = reinterpret_cast<Thread*>(arg);
		thread->run();
		return 0;
	}

	HANDLE threadHandle;
	string threadName;
};

OpenAPI::Thread::Thread():
	m_p(NULL)
{
	m_p = new ThreadPrivate();
}

OpenAPI::Thread::Thread(const std::string& threadName):
	m_p(NULL)
{
	m_p = new ThreadPrivate(threadName);
}

OpenAPI::Thread::~Thread()
{
	if(m_p != NULL)
	{
		if (m_p->threadHandle != NULL)
			CloseHandle(m_p->threadHandle);
		delete m_p;
	}
}

int OpenAPI::Thread::start()
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;
	
	m_p->threadHandle = 	(HANDLE)_beginthreadex(
                                NULL, 0, 
                                Thread::ThreadPrivate::threadProc, 
                                this, 0, NULL);
	if (m_p->threadHandle == NULL)
		return OA_ERR_SYSTEM_CALL_FAILED;

	SetThreadName(GetThreadId(m_p->threadHandle), m_p->threadName.c_str());

	return OA_ERR_NO_ERROR;
}

int OpenAPI::Thread::join()
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;

	if(WaitForSingleObject(m_p->threadHandle, INFINITE) == WAIT_OBJECT_0)
		return OA_ERR_NO_ERROR;

	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Thread::setThreadName(const std::string& threadName)
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;

	m_p->threadName = threadName;
	SetThreadName(GetThreadId(m_p->threadHandle), threadName.c_str());

	return OA_ERR_NO_ERROR;
}

int OpenAPI::Thread::getThreadName(std::string& threadName)
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;

	threadName = m_p->threadName;
	return OA_ERR_NO_ERROR;
}

int OpenAPI::Thread::getThreadId(unsigned int& threadId)
{
	if (m_p == NULL)
		return OA_ERR_OPERATION_FAILED;

	threadId = GetThreadId(m_p->threadHandle);
	return OA_ERR_NO_ERROR;
}

void OpenAPI::Thread::run() {}

unsigned int OpenAPI::Thread::GetCurrentThreadId()
{
	return ::GetCurrentThreadId();
}
