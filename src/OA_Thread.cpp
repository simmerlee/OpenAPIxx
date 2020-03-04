#include "OA_Thread.h"
#include "OA_Platform.h"
#include "OA_ErrorNumber.h"

#ifdef OA_PLT_WINDOWS
#include <Windows.h>
#include <process.h>
#include <errno.h>
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
#include <pthread.h>
#endif//OA_PLT_UNIX_FAMILY

class ThreadPrivate
{
public:
#ifdef OA_PLT_WINDOWS
    class ThreadArgs {
    public:
        OAThread *thread;
        ThreadPrivate* tp;
    };

    ThreadPrivate() : threadHandle(NULL), errnum(0) {}
    explicit ThreadPrivate(const std::string& _threadName) :
        threadHandle(NULL),
        threadName(_threadName),
        errnum(0) {}
    ~ThreadPrivate() {
        CloseHandle(threadHandle);
    }
    static unsigned int __stdcall threadProc(LPVOID arg)
    {
        ThreadArgs* args = reinterpret_cast<ThreadArgs*>(arg);
        args->thread->run();
        CloseHandle(args->tp->threadHandle);
        args->tp->threadHandle = NULL;
        return 0;
    }

    HANDLE threadHandle;
    ThreadArgs threadArgs;
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    ThreadPrivate() : errnum(0) {}
    explicit ThreadPrivate(const std::string& _threadName) :
        threadName(_threadName),
        errnum(0) {}
    ~ThreadPrivate() {}
    static void* threadProc(void* arg)
    {
        OAThread *thread = reinterpret_cast<OAThread*>(arg);
        thread->run();
        return NULL;
    }

    pthread_t threadHandle;
#endif//OA_PLT_UNIX_FAMILY

    std::string threadName;
    int32_t errnum;
};

OpenAPIxx::Thread::Thread() {
    m_p = reinterpret_cast<void*>(new ThreadPrivate());
}

OpenAPIxx::Thread::Thread(const std::string& threadName) {
    m_p = reinterpret_cast<void*>(new ThreadPrivate(threadName));
}

OpenAPIxx::Thread::~Thread() {
    if (m_p != NULL) {
        delete reinterpret_cast<ThreadPrivate*>(m_p);
    }
}

void OpenAPIxx::Thread::run() {}

int32_t OpenAPIxx::Thread::start()
{
    if (m_p == NULL)
        return OA_ERR_OPERATION_FAILED;
    ThreadPrivate* tp = reinterpret_cast<ThreadPrivate*>(m_p);

#ifdef OA_PLT_WINDOWS
    tp->threadArgs.thread = this;
    tp->threadArgs.tp = tp;
    tp->threadHandle = (HANDLE)_beginthreadex(
        NULL, 0,
        ThreadPrivate::threadProc,
        &(tp->threadArgs), 0, NULL);
    if (tp->threadHandle == NULL) {
        tp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret = pthread_create(&(tp->threadHandle), NULL,
        ThreadPrivate::threadProc, this);
    if (ret != 0) {
        tp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Thread::join()
{
    if (m_p == NULL)
        return OA_ERR_OPERATION_FAILED;
    ThreadPrivate* tp = reinterpret_cast<ThreadPrivate*>(m_p);

#ifdef OA_PLT_WINDOWS
    DWORD ret = WaitForSingleObject(tp->threadHandle, INFINITE);
    if (ret != WAIT_OBJECT_0) {
        tp->errnum = (int32_t)ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret = pthread_join(tp->threadHandle, NULL);
    if (ret != 0) {
        tp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Thread::setThreadName(const std::string& threadName)
{
    if (m_p == NULL)
        return OA_ERR_OPERATION_FAILED;
    ThreadPrivate* tp = reinterpret_cast<ThreadPrivate*>(m_p);

    tp->threadName = threadName;

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Thread::getThreadName(std::string& threadName)
{
    if (m_p == NULL)
        return OA_ERR_OPERATION_FAILED;
    ThreadPrivate* tp = reinterpret_cast<ThreadPrivate*>(m_p);

    threadName = tp->threadName;

    return OA_ERR_NO_ERROR;
}

int32_t OpenAPIxx::Thread::getLastError() {
    return reinterpret_cast<ThreadPrivate*>(m_p)->errnum;
}

/*
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

#include <linux/version.h>
#include <syscall.h>
#include <unistd.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)
#include <sys/prctl.h>
#endif

int OpenAPIxx::Thread::setThreadName(const std::string& threadName)
{
    if (m_p == NULL)
        return OA_ERR_OPERATION_FAILED;
#	if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)
    // prctl() return -1 when error (man-page)
    if (prctl(PR_SET_NAME, threadName.c_str()) == -1)
        return OA_ERR_SYSTEM_CALL_FAILED;
    else
        return OA_ERR_NO_ERROR;
#	endif// linux version new enough
    return OA_ERR_NOT_SUPPORT;
}
*/
