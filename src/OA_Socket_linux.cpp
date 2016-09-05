#include "OA_Socket.h" 
#include "OA_ErrorNumber.h"
#include "OA_Time.h"

#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

#define OA_SOCKET_INVALIDE_SOCKET_FD	(-1)
#define MAX_PORT_MASK					0xffff0000

inline string ntop(int family, void* addr)
{
	char buf[46];
	if (inet_ntop(AF_INET, addr, buf, 46) == NULL)
		return string("");
	return string(buf);
}

namespace OpenAPI
{
class SocketFdSetPrivate
{
public:
    SocketFdSetPrivate():maxFd(0){FD_ZERO(&fdSet);}

    fd_set fdSet;
    int maxFd;
};
}

OpenAPI::SocketFdSet::SocketFdSet()
{
    m_p = new SocketFdSetPrivate();
}

OpenAPI::SocketFdSet::~SocketFdSet()
{
    delete m_p;
}

void OpenAPI::SocketFdSet::clear()
{
    FD_ZERO(&(m_p->fdSet));
}

void OpenAPI::SocketFdSet::set(int fd)
{
    FD_SET(fd, &(m_p->fdSet));
    if(fd > m_p->maxFd)
        m_p->maxFd = fd;
}

bool OpenAPI::SocketFdSet::isSet(int fd)
{
    if(FD_ISSET(fd, &(m_p->fdSet)) == 0)
        return false; 
    return true;
}

void* OpenAPI::SocketFdSet::getData()
{
    return &(m_p->fdSet);
}

int OpenAPI::SocketFdSet::getMaxFd()
{
    return m_p->maxFd;
}

OpenAPI::Socket::Socket() :
	m_socketFd(OA_SOCKET_INVALIDE_SOCKET_FD),
	m_errno(0)
{}

OpenAPI::Socket::Socket(int socketFd) :
	m_socketFd(socketFd),
	m_errno(0)
{}

OpenAPI::Socket::~Socket()
{
	close();
}

int OpenAPI::Socket::bind(unsigned int port)
{
	if ((port & MAX_PORT_MASK) != 0)
		return OA_ERR_ILLEGAL_ARG;

	struct sockaddr_in addr;
	int ret;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = ::bind(m_socketFd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Socket::connect(const std::string& ip, unsigned int port)
{
	if ((port & MAX_PORT_MASK) != 0 || ip.empty())
		return OA_ERR_ILLEGAL_ARG;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.s_addr));

	int ret = ::connect(m_socketFd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Socket::send(const char* buf, 
                          unsigned int length, 
                          unsigned int* sentLength)
{
	if (buf == NULL || length == 0)
		return OA_ERR_ILLEGAL_ARG;

	int ret;
	ret = ::send(m_socketFd, buf, length, 0);
	if (ret == -1)
	{
		m_errno = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	if (sentLength != NULL)
		*sentLength = ret;

	return OA_ERR_NO_ERROR;
}

int OpenAPI::Socket::recv(char* buf, unsigned int bufSize, 
                          unsigned int* receivedLength)
{
	if (buf == NULL || bufSize == 0)
		return OA_ERR_ILLEGAL_ARG;

	int ret;
	ret = ::recv(m_socketFd, buf, bufSize, 0);
	if (ret == -1)
	{
		m_errno = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	if (receivedLength != NULL)
		*receivedLength = ret;

	return OA_ERR_NO_ERROR;
}

int OpenAPI::Socket::close()
{
	if (m_socketFd != OA_SOCKET_INVALIDE_SOCKET_FD)
	{
		if (::close(m_socketFd) != 0)
		{
			m_errno = errno;
			return OA_ERR_SYSTEM_CALL_FAILED;
		}
		else
		{
			m_socketFd = OA_SOCKET_INVALIDE_SOCKET_FD;
			return OA_ERR_NO_ERROR;
		}
	}
	else
		return OA_ERR_OPERATION_FAILED;
}

int OpenAPI::Socket::setNoneBlockModel(bool flag)
{
	int ret;
	unsigned long value;
	if (flag == true)
		value = 1;
	else
		value = 0;
	ret = ::ioctl(m_socketFd, FIONBIO, &value);
	if (ret == -1)
	{
		m_errno = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_NO_ERROR;
}

int OpenAPI::Socket::setSendBufferSize(unsigned int size)
{
	if (size == 0)
		return OA_ERR_ILLEGAL_ARG;
	if (size > 0x7fffffff)
		return OA_ERR_OUT_OF_RANGE;

	int ret = setsockopt(m_socketFd, SOL_SOCKET, SO_SNDBUF, 
                        (char*)&size, sizeof(size));
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Socket::setRecvBufferSize(unsigned int size)
{
	if (size == 0)
		return OA_ERR_ILLEGAL_ARG;
	if (size > 0x7fffffff)	// the type of value of buffer size is int
		return OA_ERR_OUT_OF_RANGE;

	int ret = setsockopt(m_socketFd, SOL_SOCKET, SO_RCVBUF, 
                        (char*)&size, sizeof(size));
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Socket::setSendTimeout(unsigned int timeoutMs)
{
	if (timeoutMs == 0)
		return OA_ERR_ILLEGAL_ARG;

	timeval tv;
	tv.tv_sec = timeoutMs / 1000;
	tv.tv_usec = timeoutMs % 1000 * 1000;
	int ret = setsockopt(m_socketFd, SOL_SOCKET, SO_SNDTIMEO, 
                        (char*)&tv, sizeof(tv));
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Socket::setRecvTimeout(unsigned int timeoutMs)
{
	if (timeoutMs == 0)
		return OA_ERR_ILLEGAL_ARG;
	
	timeval tv;
	tv.tv_sec = timeoutMs / 1000;
	tv.tv_usec = timeoutMs % 1000 * 1000;
	int ret = setsockopt(m_socketFd, SOL_SOCKET, SO_RCVTIMEO, 
                        (char*)&tv, sizeof(tv));
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::Socket::getLastError() const
{
	return m_errno;
}

void OpenAPI::Socket::setSocketFd(int socketFd)
{
	m_socketFd = socketFd;
}

int OpenAPI::Socket::getSocketFd() const
{
	return m_socketFd;
}

int OpenAPI::Socket::Select(SocketFdSet* readSet, 
                            SocketFdSet* writeSet, 
                            SocketFdSet* exceptSet, 
                            unsigned int timeout,
                            unsigned int* readyFdCounts)
{
    fd_set* rSet = NULL;
    fd_set* wSet = NULL;
    fd_set* eSet = NULL;
    if(readSet != NULL)     rSet = static_cast<fd_set*>(readSet->getData());
    if(writeSet != NULL)    wSet = static_cast<fd_set*>(writeSet->getData());
    if(exceptSet != NULL)   eSet = static_cast<fd_set*>(exceptSet->getData());
    timeval tv;
    OpenAPI::Time::SetTimeval(timeout, &tv);

    int maxFd = 0;
    int ret = ::select(maxFd + 1, rSet, wSet, eSet, &tv);
    if(ret < 0)
        return errno;
    else
    {
        if(readyFdCounts != NULL)
            *readyFdCounts = ret;
        return 0;
    }
}

OpenAPI::TCPSocket::TCPSocket() {}

OpenAPI::TCPSocket::TCPSocket(int socketFd) :
	Socket(socketFd)
{}

int OpenAPI::TCPSocket::create()
{
	m_socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_socketFd == -1)
	{
		m_errno = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_NO_ERROR;
}

int OpenAPI::TCPSocket::listen(int backlog)
{
	if (backlog <= 0)
		return OA_ERR_ILLEGAL_ARG;

	int ret = ::listen(m_socketFd, backlog);
	if (ret == 0)
		return OA_ERR_NO_ERROR;

	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::TCPSocket::accept(TCPSocket*& clientSocket, 
                               std::string* ip, 
                               unsigned int* port)
{
	struct sockaddr_in addr;
	int clientSocketFd;
	socklen_t addrLen = sizeof(addr);

	clientSocketFd = ::accept(m_socketFd, (struct sockaddr*)&addr, &addrLen);
	if (clientSocketFd == -1)
	{
		m_errno = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	clientSocket = new TCPSocket(clientSocketFd);

	if (ip != NULL)
	{
		*ip = ntop(AF_INET, &(addr.sin_addr));
	}
	if (port != NULL)
		*port = htons(addr.sin_port);

	return OA_ERR_NO_ERROR;
}

OpenAPI::UDPSocket::UDPSocket() {}

OpenAPI::UDPSocket::UDPSocket(int socketFd) :
	Socket(socketFd)
{}

int OpenAPI::UDPSocket::create()
{
	m_socketFd = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socketFd == -1)
	{
		m_errno = errno;
		return OA_ERR_SYSTEM_CALL_FAILED;
	}
	return OA_ERR_NO_ERROR;
}

int OpenAPI::UDPSocket::sendto(const char* buf, unsigned int length, 
						       const std::string& ip, unsigned int port, 
                               unsigned int *sentLength)
{
	if (buf == NULL || 
        length == 0 || 
        ip.empty()  || (port & MAX_PORT_MASK) != 0)
		return OA_ERR_ILLEGAL_ARG;

	struct sockaddr_in addr;
	int ret;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.s_addr));
	ret = ::sendto(m_socketFd, buf, length, 0, 
                  (struct sockaddr*)&addr, sizeof(addr));
	if (ret != -1)
	{
		if (sentLength != NULL)
			*sentLength = ret;
		return OA_ERR_NO_ERROR;
	}
	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

int OpenAPI::UDPSocket::recvfrom(char* buf, unsigned int bufSize, 
                                 std::string* ip, unsigned int* port, 
                                 unsigned int* receivedLength)
{
	if (buf == NULL || bufSize == 0)
		return OA_ERR_ILLEGAL_ARG;

	struct sockaddr_in addr;
	int ret;
	socklen_t addrLen = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	ret = ::recvfrom(m_socketFd, buf, bufSize, 0, 
                    (struct sockaddr*)&addr, &addrLen);
	if (ret != -1)
	{
		if (receivedLength != NULL)
			*receivedLength = ret;
		if (ip != NULL)
			*ip = ntop(AF_INET, &(addr.sin_addr));
		if (port != NULL)
			*port = ntohs(addr.sin_port);
		return OA_ERR_NO_ERROR;
	}
	m_errno = errno;
	return OA_ERR_SYSTEM_CALL_FAILED;
}

