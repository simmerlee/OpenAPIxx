#include "OA_SocketSelector.h"
#include "OA_Socket.h"
#include "OA_ErrorNumber.h"
#include <WinSock2.h>

OpenAPIxx::SocketSet::SocketSet():
    m_biggestFd(0),
    m_sockCount(0),
    m_p(NULL) {
    m_p = new fd_set();
    FD_ZERO(&m_p);
}

OpenAPIxx::SocketSet::~SocketSet() {
    delete m_p;
}

void OpenAPIxx::SocketSet::add(const Socket& sock) {
    if (FD_ISSET(*(SOCKET*)sock.getSocketHandle(), m_p) != 0) {
        m_sockCount ++;
    }
    FD_SET(*(SOCKET*)sock.getSocketHandle(), m_p);
}

void OpenAPIxx::SocketSet::del(const Socket& sock) {
    if (FD_ISSET(*(SOCKET*)sock.getSocketHandle(), m_p) != 0) {
        m_sockCount --;
    }
    FD_CLR(*(SOCKET*)sock.getSocketHandle(), m_p);
}

bool OpenAPIxx::SocketSet::isset(const Socket& sock) {
    if(FD_ISSET(*(SOCKET*)sock.getSocketHandle(), m_p) == 0)
        return false;
    else
        return true;
}

void OpenAPIxx::SocketSet::clear() {
    m_sockCount = 0;
    FD_ZERO(m_p);
}

OpenAPIxx::SocketSelector::SocketSelector():
    m_errno(0)
{}

int32_t OpenAPIxx::SocketSelector::select(
            uint32_t timeoutMs, uint32_t& readyCount) {
    int ret;
    fd_set *rs, *ws, *es;
    struct timeval tv;

    if(readSet.isEmpty() == true)
        rs = NULL;
    else
        rs = (fd_set*)readSet.getFdSet();
    if(writeSet.isEmpty() == true)
        ws = NULL;
    else
        ws = (fd_set*)writeSet.getFdSet();
    if(exceptSet.isEmpty() == true)
        es = NULL;
    else
        es = (fd_set*)exceptSet.getFdSet();

    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = timeoutMs % 1000 * 1000;
    ret = ::select(0, rs, ws, es, &tv);
    if (ret == -1) {
        m_errno = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
    readyCount = ret;

    return 0;
}


