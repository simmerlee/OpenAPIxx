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
    int32_t create();
    int32_t destroy();
	int32_t lock();
	int32_t tryLock(bool& succeed);
	int32_t unlock();
	int32_t getLastError();
private:
	void* m_p;
};

}

typedef OpenAPIxx::Lock OALock;

#endif//_OPENAPI_LOCK_H_
