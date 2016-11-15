#ifndef _OPENAPI_SOCKETSELECTOR_H_
#define _OPENAPI_SOCKETSELECTOR_H_

#include <stdint.h>
namespace OpenAPIxx {

class Socket;
class SocketSet {
public:
    SocketSet();
    ~SocketSet();
    void add(const Socket& sock);
    void del(const Socket& sock);
    bool isset(const Socket& sock);
    void clear();
    inline bool isEmpty() const { return m_sockCount == 0; }
    inline void* getFdSet() { return m_p; }
private:
    uint32_t m_biggestFd;   // useless in Windows platform
    uint32_t m_sockCount;
    void* m_p;
};

class SocketSelector {
public:
    SocketSelector();
    int32_t select(uint32_t timeoutMs, uint32_t& readyCount);
    inline int32_t getLastError() { return m_errno; }

    SocketSet readSet;
    SocketSet writeSet;
    SocketSet exceptSet;
private:
    int32_t m_errno;
};

}// namespace

typedef OpenAPIxx::SocketFdSet OASocketSet;
typedef OpenAPIxx::SocketSelector OASocketSelector;

#endif//_OPENAPI_SOCKETSELECTOR_H_
