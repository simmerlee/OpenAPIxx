#include "OA_Lock.h"
#include "OA_ErrorNumber.h"
#include "OA_Platform.h"

#ifdef OA_PLT_WINDOWS
#include <Windows.h>
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
#include <pthread.h>
#include <errno.h>
#endif//OA_PLT_UNIX_FAMILY

class LockPrivate
{
public:
#ifdef OA_PLT_WINDOWS
	LockPrivate() :mutex(NULL){}
	HANDLE mutex;
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	pthread_mutex_t mutex;
#endif//OA_PLT_UNIX_FAMILY

	int errnum;
};

OpenAPIxx::Lock::Lock():
    m_p(NULL)
{
	m_p = reinterpret_cast<void*>(new LockPrivate());
}

OpenAPIxx::Lock::~Lock()
{
	if(m_p != NULL)	{
		destroy();
		delete reinterpret_cast<LockPrivate*>(m_p);
	}
}

int OpenAPIxx::Lock::create()
{
	LockPrivate* lp = reinterpret_cast<LockPrivate*>(m_p);
	if (lp == NULL) {
		return OA_ERR_OPERATION_FAILED;
	}
#ifdef OA_PLT_WINDOWS
    lp->mutex = CreateMutex(NULL, FALSE, NULL);
	if (lp->mutex == NULL) {
		lp->errnum = ::GetLastError();
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret;
	ret = pthread_mutex_init(&(lp->mutex), NULL); // always return 0	
#endif//OA_PLT_UNIX_FAMILY
    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Lock::destroy()
{
	LockPrivate* lp = reinterpret_cast<LockPrivate*>(m_p);
	if (lp == NULL) {
		return OA_ERR_OPERATION_FAILED;
	}

#ifdef OA_PLT_WINDOWS
	if (CloseHandle(lp->mutex) == 0) {
        // If the function succeeds, the return value is nonzero.
        // --- MSDN
		lp->errnum = ::GetLastError();
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	int ret;
	ret = pthread_mutex_destroy(&(lp->mutex));
	if(ret != 0) {
		lp->errnum = ret;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_UNIX_FAMILY

	return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Lock::lock()
{
	LockPrivate* lp = reinterpret_cast<LockPrivate*>(m_p);
	if (lp == NULL) {
		return OA_ERR_OPERATION_FAILED;
	}
#ifdef OA_PLT_WINDOWS
	DWORD ret;
	ret = WaitForSingleObject(lp->mutex, INFINITE);
	if (ret == WAIT_OBJECT_0) {
		return OA_ERR_NO_ERROR;
	}
	else {
		if (ret == WAIT_FAILED) {
			lp->errnum = ::GetLastError();
		}
		else {
			// WAIT_ABANDONED or WAIT_TIMEOUT
			// 這兩種情況不在錯誤編號內
			lp->errnum = 0;
		}
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	int ret;
	ret = pthread_mutex_lock(&(lp->mutex));
	if(ret != 0) {
		lp->errnum = ret;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	else {
		return OA_ERR_NO_ERROR;
	}
#endif//OA_PLT_UNIX_FAMILY
}

int OpenAPIxx::Lock::tryLock(bool& succeed)
{
	LockPrivate* lp = reinterpret_cast<LockPrivate*>(m_p);
	if (lp == NULL) {
		return OA_ERR_OPERATION_FAILED;
	}

#ifdef OA_PLT_WINDOWS
	DWORD ret;
	ret = WaitForSingleObject(lp->mutex, 0);
	switch (ret) {
	case WAIT_OBJECT_0:
		succeed = true;
		return OA_ERR_NO_ERROR;
	case WAIT_TIMEOUT:
		succeed = false;
		return OA_ERR_NO_ERROR;
	case WAIT_ABANDONED:
		lp->errnum = 0;
		return OA_ERR_SYSTEM_CALL_FAILED;
	default:
		lp->errnum = ::GetLastError();
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	int ret;
	ret = pthread_mutex_trylock(&(lp->mutex));
	switch (ret) {
	case 0:
		succeed = true;
		return OA_ERR_NO_ERROR;
	case EBUSY:
		succeed = false;
		return OA_ERR_NO_ERROR;
	default:
		lp->errnum = ret;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_UNIX_FAMILY
}

int OpenAPIxx::Lock::unlock()
{
	LockPrivate* lp = reinterpret_cast<LockPrivate*>(m_p);
	if (lp == NULL) {
		return OA_ERR_OPERATION_FAILED;
	}

#ifdef OA_PLT_WINDOWS
	// If the function succeeds, the return value is nonzero. --MSDN
	if (ReleaseMutex(lp->mutex) == 0) {
		lp->errnum = ::GetLastError();
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	int ret;
	ret = pthread_mutex_unlock(&(lp->mutex));
	if(ret != 0) {
		lp->errnum = ret;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_UNIX_FAMILY

	return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Lock::getLastError() {
	return reinterpret_cast<LockPrivate*>(m_p)->errnum;
}

class SpinLockPrivate
{
public:
#ifdef OA_PLT_WINDOWS
    CRITICAL_SECTION criticalSection;
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    pthread_spinlock_t  spinLock;
#endif//OA_PLT_UNIX_FAMILY

    int errnum;
};

#ifdef OA_PLT_WINDOWS
#define OA_SPINLOCK_CS_SPINCOUNT		(0xffffffff)
#endif

OpenAPIxx::SpinLock::SpinLock():m_p(NULL) {
    m_p = reinterpret_cast<void*>(new SpinLockPrivate());
}

OpenAPIxx::SpinLock::~SpinLock() {
    delete reinterpret_cast<SpinLockPrivate*>(m_p);
}

int OpenAPIxx::SpinLock::create() {
    int ret;
    SpinLockPrivate* sp = reinterpret_cast<SpinLockPrivate*>(m_p);

#ifdef OA_PLT_WINDOWS
    ret = InitializeCriticalSectionAndSpinCount(
            &(sp->criticalSection), OA_SPINLOCK_CS_SPINCOUNT);

    // This function always returns a nonzero value.
    // On Windows Server 2003 and Windows XP
    // If the function fails, the return value is zero (0).
    // by MSDN

    if (ret == 0) {
        sp->errnum = GetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    ret = pthread_spin_init(
        &(sp->spinLock), PTHREAD_PROCESS_PRIVATE);
    if (ret != 0) {
        sp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

return OA_ERR_NO_ERROR;
}

int OpenAPIxx::SpinLock::destroy() {
    SpinLockPrivate* sp = reinterpret_cast<SpinLockPrivate*>(m_p);
    if (sp == NULL) {
        return OA_ERR_OPERATION_FAILED;
    }

#ifdef OA_PLT_WINDOWS
    DeleteCriticalSection(&(sp->criticalSection));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret;
    ret = pthread_spin_destroy(&(sp->spinLock));
    if (ret != 0) {
        sp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::SpinLock::lock() {
    SpinLockPrivate* sp = reinterpret_cast<SpinLockPrivate*>(m_p);
    if (sp == NULL) {
        return OA_ERR_OPERATION_FAILED;
    }

#ifdef OA_PLT_WINDOWS
    EnterCriticalSection(&(sp->criticalSection));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret;
    ret = pthread_spin_lock(&(sp->spinLock));
    if (ret != 0) {
        sp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::SpinLock::tryLock(bool& succeed) {
    SpinLockPrivate* sp = reinterpret_cast<SpinLockPrivate*>(m_p);
    if (sp == NULL) {
        return OA_ERR_OPERATION_FAILED;
    }

#ifdef OA_PLT_WINDOWS
    BOOL ret;
    ret = TryEnterCriticalSection(&(sp->criticalSection));
    if (ret != 0) {
        succeed = true;
    }
    else {
        succeed = false;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret;
    ret = pthread_spin_trylock(&(sp->spinLock));
    if (ret != 0
        && ret != EDEADLK
        && ret != EBUSY) {
        sp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
    if (ret == 0) {
        succeed = true;
    }
    else {
        succeed = false;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::SpinLock::unlock() {
    SpinLockPrivate* sp = reinterpret_cast<SpinLockPrivate*>(m_p);
    if (sp == NULL) {
        return OA_ERR_OPERATION_FAILED;
    }

#ifdef OA_PLT_WINDOWS
    LeaveCriticalSection(&(sp->criticalSection));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret;
    ret = pthread_spin_unlock(&(sp->spinLock));
    if (ret != 0) {
        sp->errnum = ret;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::SpinLock::getLastError() {
    SpinLockPrivate* sp = reinterpret_cast<SpinLockPrivate*>(m_p);
    if (sp == NULL) {
        return OA_ERR_OPERATION_FAILED;
    }

    return sp->errnum;
}
