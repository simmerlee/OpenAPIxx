#include "OA_WorkerThread.h"
#include "OA_Time.h"

#define P2PTHREAD_CHECK_PAUSED_FLAG_INTERVAL        2   // ms
#define P2PTHREAD_CHECK_PAUSE_FLAG_INTERVAL         3   // ms
#define P2PTHREAD_SLEEP_CHECK_EXIT_FALG_INTERVAL    3   // ms

OpenAPI::WorkerThread::WorkerThread() :
    m_exitFlag(false),
    m_pauseFlag(false),
    m_pausedFlag(false)
{}

OpenAPI::WorkerThread::WorkerThread(const std::string& threadName) :
    OAThread(threadName),
    m_exitFlag(false),
    m_pauseFlag(false),
    m_pausedFlag(false)
{}

OpenAPI::WorkerThread::~WorkerThread()
{}

void OpenAPI::WorkerThread::run()
{}

void OpenAPI::WorkerThread::waitPaused()
{
    while (m_pausedFlag == false)
        OATime::SleepMs(P2PTHREAD_CHECK_PAUSED_FLAG_INTERVAL);
}

bool OpenAPI::WorkerThread::shouldKeepRunning()
{
    if (m_exitFlag == true)
        return false;
    if (m_pauseFlag == true)
    {
        // 進入暫停狀態
        m_pausedFlag = true;
        while (m_pauseFlag == true)
        {
            if (m_exitFlag == true)
                return false;
			OATime::SleepMs(P2PTHREAD_CHECK_PAUSE_FLAG_INTERVAL);
        }

        // 退出暫停狀態
        m_pausedFlag = false;
    }
    return true;
}

void OpenAPI::WorkerThread::sleepUnlessExit(unsigned int time)
{
    unsigned long long goal = OATime::TickCount64() + time;
    while (m_exitFlag == false)
    {
        if (OATime::TickCount64() < goal)
			OATime::SleepMs(P2PTHREAD_SLEEP_CHECK_EXIT_FALG_INTERVAL);
        else
            break;
    }
}
