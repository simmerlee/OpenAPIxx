#include "OA_WorkerThread.h"
#include "OA_Time.h"

#define P2PTHREAD_CHECK_PAUSED_FLAG_INTERVAL        2   // ms
#define P2PTHREAD_CHECK_PAUSE_FLAG_INTERVAL         3   // ms
#define P2PTHREAD_SLEEP_CHECK_EXIT_FALG_INTERVAL    3   // ms

typedef OpenAPIxx::Time OATime;

OpenAPIxx::WorkerThread::WorkerThread() :
    m_exitFlag(false),
    m_pauseFlag(false),
    m_pausedFlag(false)
{}

OpenAPIxx::WorkerThread::WorkerThread(const std::string& threadName) :
    Thread(threadName),
    m_exitFlag(false),
    m_pauseFlag(false),
    m_pausedFlag(false)
{}

OpenAPIxx::WorkerThread::~WorkerThread()
{}

void OpenAPIxx::WorkerThread::run()
{}

void OpenAPIxx::WorkerThread::waitPaused()
{
    while (m_pausedFlag == false)
        OATime::Msleep(P2PTHREAD_CHECK_PAUSED_FLAG_INTERVAL);
}

bool OpenAPIxx::WorkerThread::shouldKeepRunning()
{
    if (m_exitFlag == true)
        return false;
    if (m_pauseFlag == true)
    {
        // ßMÈë•ºÍ£ î‘B
        m_pausedFlag = true;
        while (m_pauseFlag == true)
        {
            if (m_exitFlag == true)
                return false;
            OATime::Msleep(P2PTHREAD_CHECK_PAUSE_FLAG_INTERVAL);
        }

        // ÍË³ö•ºÍ£ î‘B
        m_pausedFlag = false;
    }
    return true;
}

void OpenAPIxx::WorkerThread::sleepUnlessExit(unsigned int time)
{
    unsigned long long goal = OATime::TickCount64() + time;
    while (m_exitFlag == false)
    {
        if (OATime::TickCount64() < goal)
            OATime::Msleep(P2PTHREAD_SLEEP_CHECK_EXIT_FALG_INTERVAL);
        else
            break;
    }
}
