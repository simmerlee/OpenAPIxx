#ifndef _OPENAPI_LOCK_H_
#define _OPENAPI_LOCK_H_

#include <stdint.h>

namespace OpenAPIxx
{

class Lock
{
public:
	Lock();
	~Lock();
    int create();
    int destroy();
	int lock();
	int tryLock(bool& succeed);
	int unlock();
	int getLastError();
private:
	void* m_p;
};

class SpinLock {
public:
    SpinLock();
    ~SpinLock();
    int create();
    int destroy();
    int lock();
    int tryLock(bool& succeed);
    int unlock();
    int getLastError();
private:
    void* m_p;
};

}

typedef OpenAPIxx::Lock OALock;
typedef OpenAPIxx::SpinLock OASpinLock;

#endif//_OPENAPI_LOCK_H_
