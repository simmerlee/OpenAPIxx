#ifndef _OPENAPIXX_WORKER_THREAD_H_
#define _OPENAPIXX_WORKER_THREAD_H_

#include "OA_Thread.h"

namespace OpenAPIxx {

class WorkerThread : public OpenAPIxx::Thread
{
public:
    WorkerThread();
    WorkerThread(const std::string& threadName);
    virtual ~WorkerThread();
    virtual void run();
    inline void setExitFlagTrue() { m_exitFlag = true; }
    inline void setPauseFlagTrue() { m_pauseFlag = true; }
    inline void setPauseFlagFalse() { m_pauseFlag = false; }
    void waitPaused();  // puasedFlag�Þ���󣬵ȴ�����������ͣ
    inline bool shouldExit() { return m_exitFlag; }
protected:
    bool shouldKeepRunning();
    void sleepUnlessExit(unsigned int time);
private:
    volatile bool m_exitFlag;       // �Ƿ���Ҫ�˳�
    volatile bool m_pauseFlag;      // �Ƿ���Ҫ��ͣ
    volatile bool m_pausedFlag;     // �Ƿ��ѽ���ͣ
};

}// namespace OpenAPIxx

typedef OpenAPIxx::WorkerThread OAWorkerThread;

#endif//_OPENAPIXX_WORKER_THREAD_H_
