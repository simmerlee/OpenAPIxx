#include "OA_Lock.h"
#include "OA_ErrorNumber.h"
#include <errno.h>
#include <pthread.h>

class OpenAPI::Lock::LockPrivate
{
public:
	pthread_mutex_t mutex;
};

OpenAPI::Lock::Lock()
{
	m_p = new OpenAPI::Lock::LockPrivate();
	pthread_mutex_init(&(m_p->mutex), NULL);
}

OpenAPI::Lock::~Lock()
{
	if(m_p != NULL)
	{
		pthread_mutex_destroy(&(m_p->mutex));
		delete m_p;
	}
}

int OpenAPI::Lock::lock()
{
	if(m_p != NULL)
	{
		if(pthread_mutex_lock(&(m_p->mutex)) == 0)
			return OA_ERR_NO_ERROR;
		else
			return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_OPERATION_FAILED;
}

int OpenAPI::Lock::tryLock(bool& succeed)
{
	int ret = 0;
	if(m_p != NULL)
	{
		ret = pthread_mutex_trylock(&(m_p->mutex));
		if(ret == 0)
		{
			succeed = true;
			return OA_ERR_NO_ERROR;
		}
		if(ret == EBUSY)
		{
			succeed = false;
			return OA_ERR_NO_ERROR;
		}
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_OPERATION_FAILED;
}

int OpenAPI::Lock::unlock()
{
	if(m_p != NULL)
	{
		if(pthread_mutex_unlock(&(m_p->mutex)) == 0)
			return OA_ERR_NO_ERROR;
		else
			return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_OPERATION_FAILED;
}
