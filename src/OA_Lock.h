#ifndef _OPENAPI_LOCK_H_
#define _OPENAPI_LOCK_H_

namespace OpenAPI
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
private:
    class LockPrivate;
    LockPrivate* m_p;
};

}

typedef OpenAPI::Lock OALock;

#endif//_OPENAPI_LOCK_H_
