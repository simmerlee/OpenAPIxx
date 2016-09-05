#include "OA_Lock.h"
#include "OA_ErrorNumber.h"
#include <new>
#include <iostream>
#include <Windows.h>

class OpenAPI::Lock::LockPrivate
{
public:
	LockPrivate():mutex(NULL), createFlag(false){}

	HANDLE mutex;
    bool createFlag;
};

OpenAPI::Lock::Lock():
    m_p(NULL)
{
	m_p = new OpenAPI::Lock::LockPrivate();
}

OpenAPI::Lock::~Lock()
{
	if(m_p != NULL)
	{
        if (m_p->createFlag == true)
            CloseHandle(m_p->mutex);
		delete m_p;
	}
}

int OpenAPI::Lock::create()
{
    m_p->mutex = CreateMutex(NULL, FALSE, NULL);
    if (m_p->mutex != NULL)
        return OA_ERR_SYSTEM_CALL_FAILED;

    return OA_ERR_NO_ERROR;
}

int OpenAPI::Lock::destroy()
{
    if (m_p != NULL)
    {
        if (m_p->createFlag == false)
            return OA_ERR_OPERATION_FAILED;

        m_p->createFlag = false;
        if (CloseHandle(m_p->mutex) == 0)
            return OA_ERR_SYSTEM_CALL_FAILED;
        return OA_ERR_NO_ERROR;
    }
    return OA_ERR_OPERATION_FAILED;
}

int OpenAPI::Lock::lock()
{
	if(m_p != NULL)
	{
		if(WaitForSingleObject(m_p->mutex, INFINITE) == WAIT_OBJECT_0)
			return OA_ERR_NO_ERROR;
		else
			return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_OPERATION_FAILED;
}

int OpenAPI::Lock::tryLock(bool& succeed)
{
	DWORD ret;
	if(m_p != NULL)
	{
		ret = WaitForSingleObject(m_p->mutex, 0);
		if(ret == WAIT_OBJECT_0)
		{
			succeed = true;
			return OA_ERR_NO_ERROR;
		}
		else if(ret == WAIT_TIMEOUT)
		{
			succeed = false;
			return OA_ERR_NO_ERROR;
		}
		else
			return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_OPERATION_FAILED;
}

int OpenAPI::Lock::unlock()
{
	if(m_p != NULL)
	{
		// If the function succeeds, the return value is nonzero. --MSDN
		if(ReleaseMutex(m_p->mutex) != 0)	
			return OA_ERR_NO_ERROR;
		else
			return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_OPERATION_FAILED;
}

