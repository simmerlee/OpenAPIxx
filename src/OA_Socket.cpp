// 說明：
// Windows平台下的send, recv, sendto, recvfrom函數的
// 返回值和bufSize都為int類型，而linux下為ssize_t和size_t類型
// 作為統一，OA系列的所有返回值為int，bufSize為size_t

#include "OA_Socket.h"
#include "OA_ErrorNumber.h"
#include "OA_Platform.h"
#include <cstring>

#ifdef OA_PLT_WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif//OA_PLT_UNIX_FAMILY

#ifdef OA_PLT_WINDOWS
#define OA_SOCKET_INVALIDE_SOCKET_FD    INVALID_SOCKET
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
#define OA_SOCKET_INVALIDE_SOCKET_FD	(-1)
#endif//OA_PLT_UNIX_FAMILY
#define MAX_PORT_MASK					0xffff0000


#ifdef OA_PLT_WINDOWS
class WindowsWSA
{
public:
    WindowsWSA()
    {
        WORD wVersionRequested;
        WSADATA wsaData;

        wVersionRequested = MAKEWORD(2, 2);
        WSAStartup(wVersionRequested, &wsaData);
    }
    ~WindowsWSA()
    {
        WSACleanup();
    }
};

static WindowsWSA wsa;
#endif//OA_PLT_WINDOWS

inline std::string ntop(int family, void* addr)
{
    char buf[46];
    if (inet_ntop(family, addr, buf, 46) == NULL)
        return std::string("");
    return std::string(buf);
}

uint64_t bswap64(uint64_t n)
{
    uint64_t r;
    size_t i = 0, j = 7;
    char *p1 = (char *)&n;
    char *p2 = (char *)&r;
    while (i < 8)
        p2[i++] = p1[j--];
    return r;
}

union ByteOrderUnion
{
    uint32_t data1;
    char data2[4];
};

class ByteOrderUnionData
{
public:
    ByteOrderUnionData()
    {
        data.data2[0] = 0x00;
        data.data2[1] = 0x00;
        data.data2[2] = 0x00;
        data.data2[3] = 0x01;
    }
    ByteOrderUnion data;
};

namespace
{
    ByteOrderUnionData byteOrderUnionData;
}

class ByteOrder
{
public:
    static bool IsBigEndian() { return byteOrderUnionData.data.data1 == 1; }
    static bool IsLittleEndian() { return byteOrderUnionData.data.data1 != 1; }
};

class SocketPrivate {
public:
#ifdef OA_PLT_WINDOWS
    SocketPrivate(bool reserveFlag_) :
        errnum(0), socket(NULL), reserveFlag(reserveFlag_){}

    SOCKET socket;
    int errnum;
    bool reserveFlag;
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	SocketPrivate(bool reserveFlag_) :
        errnum(0), socket(OA_SOCKET_INVALIDE_SOCKET_FD), 
        reserveFlag(reserveFlag_) {}

	int socket;
	int errnum;
    bool reserveFlag;
#endif//OA_PLT_UNIX_FAMILY
};

OpenAPIxx::Socket::Socket() :
    m_p(NULL)
{
    m_p = new SocketPrivate(false);
}

OpenAPIxx::Socket::Socket(const void* socketFd, bool reserveFlag) :
    m_p(NULL)
{
    m_p = new SocketPrivate(reserveFlag);

#ifdef OA_PLT_WINDOWS
    ((SocketPrivate*)m_p)->socket = *(SOCKET*)socketFd;
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    ((SocketPrivate*)m_p)->socket = *(int*)socketFd;
#endif//OA_PLT_UNIX_FAMILY
}

OpenAPIxx::Socket::~Socket()
{
    SocketPrivate* sp = reinterpret_cast<SocketPrivate*>(m_p);
    if (sp == NULL) {
        return;
    }
    // reserveFlag為false時
    // 才釋放socket所佔用的系統資源
    if (sp->reserveFlag == false) {
        OASocket::close();
    }
    delete (SocketPrivate*)m_p;
}

int OpenAPIxx::Socket::bind(uint32_t port)
{
    if ((port & MAX_PORT_MASK) != 0)
        return OA_ERR_ILLEGAL_ARG;

    struct sockaddr_in addr;
    int ret;
    SocketPrivate* sp = (SocketPrivate*)m_p;
    uint16_t port16 = static_cast<uint16_t>(port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port16);

#ifdef OA_PLT_WINDOWS
    addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif//OA_PLT_UNIX_FAMILY 

    ret = ::bind(sp->socket, 
                 (struct sockaddr*)&addr, sizeof(addr));
    if (ret == 0)
        return OA_ERR_NO_ERROR;

#ifdef OA_PLT_WINDOWS
    sp->errnum = WSAGetLastError();
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	sp->errnum = errno;
#endif//OA_PLT_UNIX_FAMILY 
    return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPIxx::Socket::connect(const std::string& ip, uint32_t port)
{
    if ((port & MAX_PORT_MASK) != 0 || ip.empty())
        return OA_ERR_ILLEGAL_ARG;

    struct sockaddr_in addr;
    SocketPrivate* sp = (SocketPrivate*)m_p;
    uint16_t port16 = static_cast<uint16_t>(port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port16);

#ifdef OA_PLT_WINDOWS
    inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.S_un.S_addr));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.s_addr));
#endif//OA_PLT_UNIX_FAMILY 

    int ret = ::connect(sp->socket, 
                            (struct sockaddr*)&addr, sizeof(addr));
    if (ret == 0)
        return OA_ERR_NO_ERROR;

#ifdef OA_PLT_WINDOWS
    sp->errnum = WSAGetLastError();
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	sp->errnum = errno;
#endif//OA_PLT_UNIX_FAMILY 
    return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPIxx::Socket::connect(const std::string & ip,
                uint32_t port, uint32_t timeout)
{
    int ret;
    OASelector selector;
    size_t readyCount;

    this->setNoneBlockModel(true);
    // 非阻塞connect一般都會失敗
    // 這裡不檢查返回值
    this->connect(ip, port);
    selector.clearAllSet();
    selector.writeSetSet(this);
    ret = selector.select(timeout, &readyCount);
    if (ret != 0) {
        return -1;
    }
    if (readyCount == 0 ||
        // 理論上不應該發生
        selector.writeSetIsset(this) == false) {
        return -1;
    }
    this->setNoneBlockModel(false);
    return 0;
}

int OpenAPIxx::Socket::send(
                const char* buf, size_t length, size_t* sentLength)
{
    if (buf == NULL || length == 0)
        return OA_ERR_ILLEGAL_ARG;

    SocketPrivate* sp = (SocketPrivate*)m_p;

#ifdef OA_PLT_WINDOWS
    if (length > MAXINT) {
        return OA_ERR_OUT_OF_RANGE;
    }
    int length2 = static_cast<int>(length);
    int ret = ::send(sp->socket, buf, length2, 0);
    if (ret == -1) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#else
    ssize_t ret = ::send(sp->socket, buf, length, 0);
    if (ret == -1) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif

    if (sentLength != NULL)
        *sentLength = ret;

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::recv(
                char* buf, size_t bufSize, size_t* receivedLength)
{
    if (buf == NULL || bufSize == 0)
        return OA_ERR_ILLEGAL_ARG;

    SocketPrivate* sp = (SocketPrivate*)m_p;
    
#ifdef OA_PLT_WINDOWS
    if (bufSize > MAXINT) {
        return OA_ERR_OUT_OF_RANGE;
    }
    int bufSize2 = static_cast<int>(bufSize);
    int ret = ::recv(sp->socket, buf, bufSize2, 0);
    if (ret == -1) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#else
    ssize_t ret = ::recv(sp->socket, buf, bufSize, 0);
    if (ret == -1) {
		sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS

    if (receivedLength != NULL)
        *receivedLength = ret;

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::close()
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    if (sp->socket == OA_SOCKET_INVALIDE_SOCKET_FD) {
        return OA_ERR_OPERATION_FAILED;
    }

#ifdef OA_PLT_WINDOWS
    if (::closesocket(sp->socket) != 0) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	if (::close(sp->socket) != 0) {
	    sp->errnum = errno;
	    return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_UNIX_FAMILY 

    sp->socket = OA_SOCKET_INVALIDE_SOCKET_FD;
    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::setReserveFlag(bool flag) {
    SocketPrivate* sp = reinterpret_cast<SocketPrivate*>(m_p);
    if (sp == NULL) {
        return OA_ERR_OPERATION_FAILED;
    }
    sp->reserveFlag = flag;
    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::setNoneBlockModel(bool flag)
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    int ret;
    unsigned long value;
    if (flag == true) {
        value = 1;
    }
    else {
        value = 0;
    }
#ifdef OA_PLT_WINDOWS
    ret = ::ioctlsocket(sp->socket, FIONBIO, &value);
    if (ret == -1)
    {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    ret = ::ioctl(sp->socket, FIONBIO, &value);
    if (ret == -1)
    {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::setSendBufferSize(size_t size)
{
    if (size == 0)
        return OA_ERR_ILLEGAL_ARG;
    if (size > 0x7fffffff)
        return OA_ERR_OUT_OF_RANGE;

    SocketPrivate* sp = (SocketPrivate*)m_p;

    int ret = setsockopt(sp->socket, SOL_SOCKET, SO_SNDBUF,
        (char*)&size, sizeof(size));

#ifdef OA_PLT_WINDOWS
    if (ret != 0) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    if (ret != 0) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
}
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::setRecvBufferSize(size_t size)
{
    if (size == 0)
        return OA_ERR_ILLEGAL_ARG;
    if (size > 0x7fffffff)	// the type of value of buffer size is int
        return OA_ERR_OUT_OF_RANGE;

    SocketPrivate* sp = (SocketPrivate*)m_p;
    int ret = setsockopt(sp->socket, SOL_SOCKET, SO_RCVBUF,
        (char*)&size, sizeof(size));

#ifdef OA_PLT_WINDOWS
    if (ret != 0) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    if (ret != 0) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY
    
    return OA_ERR_NO_ERROR;            
}

int OpenAPIxx::Socket::setSendTimeout(uint32_t timeoutMs)
{
    if (timeoutMs == 0)
        return OA_ERR_ILLEGAL_ARG;

    SocketPrivate* sp = (SocketPrivate*)m_p;

#ifdef OA_PLT_WINDOWS
    int ret = setsockopt(sp->socket, SOL_SOCKET, SO_SNDTIMEO,
        (char*)&timeoutMs, sizeof(timeoutMs));
    if (ret != 0) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = timeoutMs % 1000 * 1000;
    int ret = setsockopt(sp->socket, SOL_SOCKET, SO_SNDTIMEO,
        (char*)&tv, sizeof(tv));
    if (ret != 0) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::setRecvTimeout(uint32_t timeoutMs)
{
    if (timeoutMs == 0)
        return OA_ERR_ILLEGAL_ARG;

    SocketPrivate* sp = (SocketPrivate*)m_p;

#ifdef OA_PLT_WINDOWS
    int ret = setsockopt(sp->socket, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&timeoutMs, sizeof(timeoutMs));
    if (ret != 0) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = timeoutMs % 1000 * 1000;
    int ret = setsockopt(sp->socket, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&tv, sizeof(tv));
    if (ret != 0) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::Socket::getSocketName(uint32_t& port)
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    struct sockaddr_in addr;

#ifdef OA_PLT_WINDOWS
    int addrLen = sizeof(addr);
    if (getsockname(sp->socket, (struct sockaddr*)&addr, &addrLen) == -1) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    socklen_t addrLen = sizeof(addr);
    if (getsockname(sp->socket, (struct sockaddr*)&addr, &addrLen) == -1) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    port = static_cast<uint32_t>(ntohs(addr.sin_port));

    return OA_ERR_NO_ERROR;
}

const void* OpenAPIxx::Socket::getSocketFd() const {
    return &(((SocketPrivate*)(m_p))->socket);
}

int OpenAPIxx::Socket::getLastError() const
{
    return ((SocketPrivate*)(m_p))->errnum;
}

int OpenAPIxx::Socket::GetHostByName(const std::string & domainName,
    std::string & ip)
{
    struct addrinfo hint;
    struct addrinfo* result;
    hint.ai_family = AF_INET;
    int ret;
    char buf[20];

    memset(&hint, 0, sizeof(hint));
    ret = getaddrinfo(domainName.c_str(), NULL, &hint, &result);
    if (ret != 0) {
        return -1;
    }
    inet_ntop(AF_INET,
        &(((struct sockaddr_in*)(result->ai_addr))->sin_addr),
        buf, 20);
    freeaddrinfo(result);
    ip.assign(buf);

    return 0;
}

uint16_t OpenAPIxx::Socket::ntoh16(uint16_t num) { return ntohs(num); }
uint16_t OpenAPIxx::Socket::hton16(uint16_t num) { return htons(num); }
uint32_t OpenAPIxx::Socket::ntoh32(uint32_t num) { return ntohl(num); }
uint32_t OpenAPIxx::Socket::hton32(uint32_t num) { return htonl(num); }

inline uint64_t OpenAPIxx::Socket::ntoh64(uint64_t num)
{
    if (ByteOrder::IsLittleEndian() == true)
    {
        uint64_t temp;
        char* pi = reinterpret_cast<char*>(&num);
        char* pj = reinterpret_cast<char*>(&temp);
        pj[0] = pi[7];
        pj[1] = pi[6];
        pj[2] = pi[5];
        pj[3] = pi[4];
        pj[4] = pi[3];
        pj[5] = pi[2];
        pj[6] = pi[1];
        pj[7] = pi[0];
        return temp;
    }
    return num;
}

uint64_t OpenAPIxx::Socket::hton64(uint64_t num)
{
    return ntoh64(num);
}

OpenAPIxx::TCPSocket::TCPSocket() {}

OpenAPIxx::TCPSocket::TCPSocket(const void* socketFd, bool reserveFlag) :
    Socket(socketFd, reserveFlag)
{}

int OpenAPIxx::TCPSocket::create()
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    sp->socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sp->socket == OA_SOCKET_INVALIDE_SOCKET_FD)
    {
#ifdef OA_PLT_WINDOWS
        sp->errnum = WSAGetLastError();
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
		sp->errnum = errno;
#endif//OA_PLT_UNIX_FAMILY 
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::TCPSocket::listen(int backlog)
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    if (backlog <= 0)
        return OA_ERR_ILLEGAL_ARG;

    int ret = ::listen(sp->socket, backlog);
    if (ret == 0)
        return OA_ERR_NO_ERROR;

#ifdef OA_PLT_WINDOWS
    sp->errnum = WSAGetLastError();
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	sp->errnum = errno;
#endif//OA_PLT_UNIX_FAMILY 

    return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPIxx::TCPSocket::accept(TCPSocket*& clientSocket,
    std::string* ip, uint32_t* port)
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    struct sockaddr_in addr;

#ifdef OA_PLT_WINDOWS
    SOCKET clientSocketFd;
    int addrLen = sizeof(addr);

    clientSocketFd = ::accept(sp->socket, (struct sockaddr*)&addr, &addrLen);
    if (clientSocketFd == OA_SOCKET_INVALIDE_SOCKET_FD)
    {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
	int clientSocketFd;
	socklen_t addrLen = sizeof(addr);

	clientSocketFd = ::accept(sp->socket, (struct sockaddr*)&addr, &addrLen);
	if (clientSocketFd == OA_SOCKET_INVALIDE_SOCKET_FD)
	{
        sp->errnum = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
#endif//OA_PLT_UNIX_FAMILY 

    clientSocket = new TCPSocket(&clientSocketFd, false);

    if (ip != NULL)
    {
        *ip = ntop(AF_INET, &(addr.sin_addr));
    }
    if (port != NULL)
        *port = static_cast<uint32_t>(htons(addr.sin_port));

    return OA_ERR_NO_ERROR;
}

OpenAPIxx::UDPSocket::UDPSocket() {}

OpenAPIxx::UDPSocket::UDPSocket(const void* socketFd, bool reserveFlag) :
    Socket(socketFd, reserveFlag)
{}

int OpenAPIxx::UDPSocket::create()
{
    SocketPrivate* sp = (SocketPrivate*)m_p;
    sp->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sp->socket == OA_SOCKET_INVALIDE_SOCKET_FD)
    {
#ifdef OA_PLT_WINDOWS
        sp->errnum = WSAGetLastError();
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
		sp->errnum = errno;
#endif//OA_PLT_UNIX_FAMILY 
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::UDPSocket::sendto(const char* buf, size_t length,
    const std::string& ip, uint32_t port,
    size_t *sentLength)
{
    if (buf == NULL
        || length == 0
        || ip.empty()
        || (port & MAX_PORT_MASK) != 0) {
        return OA_ERR_ILLEGAL_ARG;
    }

#ifdef OA_PLT_WINDOWS
    if (length > MAXINT) {
        return OA_ERR_OUT_OF_RANGE;
    }
    int length2 = static_cast<int>(length);
    int ret;
    struct sockaddr_in addr;
    SocketPrivate* sp = (SocketPrivate*)m_p;
    uint16_t port16 = static_cast<uint16_t>(port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port16);
    inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.S_un.S_addr));///////////////////////////////////////
    ret = ::sendto(sp->socket, buf, length2, 0,
        (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    ssize_t ret;
    struct sockaddr_in addr;
    SocketPrivate* sp = (SocketPrivate*)m_p;
    uint16_t port16 = static_cast<uint16_t>(port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port16);
    inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.s_addr));
    ret = ::sendto(sp->socket, buf, length, 0,
        (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    if (sentLength != NULL)
        *sentLength = static_cast<size_t>(ret);
    return OA_ERR_NO_ERROR;
}

int OpenAPIxx::UDPSocket::recvfrom(char* buf, size_t bufSize,
    std::string* ip, uint32_t* port,
    size_t* receivedLength)
{
    if (buf == NULL || bufSize == 0)
        return OA_ERR_ILLEGAL_ARG;

#ifdef OA_PLT_WINDOWS
    if (bufSize > MAXINT) {
        return OA_ERR_OUT_OF_RANGE;
    }
    SocketPrivate* sp = (SocketPrivate*)m_p;
    struct sockaddr_in addr;
    int addrLen = sizeof(addr);
    int ret;
    int bufSize2 = static_cast<int>(bufSize);

    memset(&addr, 0, sizeof(addr));
    ret = ::recvfrom(sp->socket, buf, bufSize2, 0,
        (struct sockaddr*)&addr, &addrLen);
    if (ret == -1) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    SocketPrivate* sp = (SocketPrivate*)m_p;
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    ssize_t ret;

    memset(&addr, 0, sizeof(addr));
    ret = ::recvfrom(sp->socket, buf, bufSize, 0,
        (struct sockaddr*)&addr, &addrLen);
    if (ret == -1) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY

    if (receivedLength != NULL)
        *receivedLength = static_cast<size_t>(ret);
    if (ip != NULL)
        *ip = ntop(AF_INET, &(addr.sin_addr));
    if (port != NULL)
        *port = static_cast<uint32_t>(ntohs(addr.sin_port));
    return OA_ERR_NO_ERROR;
}

class SelectorPrivate {
public:
    SelectorPrivate():errnum(0)
    {
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&excpSet);
    }

    fd_set readSet;
    fd_set writeSet;
    fd_set excpSet;
    int errnum;
#ifdef OA_PLT_UNIX_FAMILY
    int maxFd;
#endif
};

OpenAPIxx::Selector::Selector() :
    m_p(NULL) 
{
    m_p = new SelectorPrivate();
}

OpenAPIxx::Selector::~Selector() {
    if (m_p != NULL) {
        delete (SelectorPrivate*)m_p;
    }
}

void OpenAPIxx::Selector::clearAllSet()
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;
    FD_ZERO(&(sp->readSet));
    FD_ZERO(&(sp->writeSet));
    FD_ZERO(&(sp->excpSet));
#ifdef OA_PLT_UNIX_FAMILY
    sp->maxFd = 0;
#endif
}

void OpenAPIxx::Selector::readSetSet(const OpenAPIxx::Socket* socket)
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;
#ifdef OA_PLT_WINDOWS
    FD_SET(*(const SOCKET*)(socket->getSocketFd()), &(sp->readSet));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int fd = *(const int*)(socket->getSocketFd());
    if(fd > sp->maxFd) {
        sp->maxFd = fd;
    }
    FD_SET(fd, &(sp->readSet));
#endif//OA_PLT_UNIX_FAMILY 
}

bool    OpenAPIxx::Selector::readSetIsset(const OpenAPIxx::Socket* socket)
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;

#ifdef OA_PLT_WINDOWS
    int ret = FD_ISSET(*(const SOCKET*)(socket->getSocketFd()),
                    &(sp->readSet));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret = FD_ISSET(*(const int*)(socket->getSocketFd()),
        &(sp->readSet));
#endif//OA_PLT_UNIX_FAMILY 
    if(ret != 0)
    {
        return true;
    }
    return false;
}

void OpenAPIxx::Selector::writeSetSet(const OpenAPIxx::Socket* socket)
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;
#ifdef OA_PLT_WINDOWS
    FD_SET(*(const SOCKET*)(socket->getSocketFd()), &(sp->writeSet));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int fd = *(const int*)(socket->getSocketFd());
    if (fd > sp->maxFd) {
        sp->maxFd = fd;
    }
    FD_SET(fd, &(sp->writeSet));
#endif//OA_PLT_UNIX_FAMILY 
}

bool    OpenAPIxx::Selector::writeSetIsset(const OpenAPIxx::Socket* socket)
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;

#ifdef OA_PLT_WINDOWS
    int ret = FD_ISSET(*(const SOCKET*)(socket->getSocketFd()),
        &(sp->writeSet));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret = FD_ISSET(*(const int*)(socket->getSocketFd()),
        &(sp->writeSet));
#endif//OA_PLT_UNIX_FAMILY 

    if (ret != 0)
    {
        return true;
    }
    return false;
}

void OpenAPIxx::Selector:: excpSetSet(const OpenAPIxx::Socket* socket)
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;
#ifdef OA_PLT_WINDOWS
    FD_SET(*(const SOCKET*)(socket->getSocketFd()), &(sp->excpSet));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int fd = *(const int*)(socket->getSocketFd());
    if (fd > sp->maxFd) {
        sp->maxFd = fd;
    }
    FD_SET(fd, &(sp->excpSet));
#endif//OA_PLT_UNIX_FAMILY 
}

bool    OpenAPIxx::Selector::excpSetIsset(const OpenAPIxx::Socket* socket)
{
    SelectorPrivate* sp = (SelectorPrivate*)m_p;

#ifdef OA_PLT_WINDOWS
    int ret = FD_ISSET(*(const SOCKET*)(socket->getSocketFd()),
        &(sp->excpSet));
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    int ret = FD_ISSET(*(const int*)(socket->getSocketFd()),
        &(sp->excpSet));
#endif//OA_PLT_UNIX_FAMILY 

    if (ret != 0) {
        return true;
    }
    return false;
}

int OpenAPIxx::Selector::select(uint32_t timeoutMs, size_t* readyCount)
{
    if (readyCount == NULL) {
        return OA_ERR_ILLEGAL_ARG;
    }
    SelectorPrivate* sp = (SelectorPrivate*)m_p;
    timeval tv;
    timeval* tvp;
    int ret;

    if(timeoutMs == OA_SELECTOR_MAX_TIMEOUT) {
        tvp = NULL;
    }
    else {
        tv.tv_sec = timeoutMs / 1000L;
        tv.tv_usec = timeoutMs % 1000L * 1000L;
        tvp = &tv;
    }

#ifdef OA_PLT_WINDOWS
    ret = ::select(0, &(sp->readSet), &(sp->writeSet), &(sp->excpSet), tvp);
    if (ret < 0) {
        sp->errnum = WSAGetLastError();
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_WINDOWS
#ifdef OA_PLT_UNIX_FAMILY
    ret = ::select(sp->maxFd+1, 
                    &(sp->readSet), &(sp->writeSet), &(sp->excpSet), tvp);
    if (ret < 0) {
        sp->errnum = errno;
        return OA_ERR_SYSTEM_CALL_FAILED;
    }
#endif//OA_PLT_UNIX_FAMILY 
    
    *readyCount = (uint32_t)ret;
    return 0;
}

int OpenAPIxx::Selector::getLastError()
{
    return ((SelectorPrivate*)m_p)->errnum;
}

