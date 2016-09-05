#ifndef _OPENAPI_SOCKET_H_
#define _OPENAPI_SOCKET_H_

#include <string>

namespace OpenAPI
{

class SocketFdSetPrivate;
class SocketFdSet
{
public:
    SocketFdSet();
    ~SocketFdSet();
    void clear();
    void set(int fd);
    bool isSet(int fd);
    void* getData();
    int getMaxFd();
private:
    SocketFdSetPrivate* m_p;
};

class Socket
{
public:
	Socket();
	explicit Socket(int socketFd);
	virtual ~Socket();
	virtual int create() = 0;
	int bind(unsigned int port);
	int connect(const std::string& ip, unsigned int port);
	int send(const char* buf, unsigned int length, unsigned int* sentLength);
	int recv(char* buf, unsigned int bufSize, unsigned int* receivedLength);
	int close();
	int setNoneBlockModel(bool flag);
	int setSendBufferSize(unsigned int size);
	int setRecvBufferSize(unsigned int size);
	int setSendTimeout(unsigned int timeoutMs);
	int setRecvTimeout(unsigned int timeoutMs);
	void setSocketFd(int socketFd);
	int getSocketFd() const;
	int getLastError() const;

    static int Select(
        SocketFdSet* readSet, 
        SocketFdSet* writeSet, 
        SocketFdSet* exceptSet, 
        unsigned int timeout,
        unsigned int* readyFdCounts);

protected:
	int m_socketFd;
	int m_errno;
};

class TCPSocket : public Socket
{
public:
	TCPSocket();
	explicit TCPSocket(int socketFd);
	int create();
	int listen(int backlog);
	int accept(TCPSocket*& clientSocket, std::string* ip, unsigned int* port);
};

class UDPSocket : public Socket
{
public:
	UDPSocket();
	explicit UDPSocket(int socketFd);
	int create();
	int sendto(const char* buf, unsigned int length, const std::string& ip, unsigned int port, unsigned int *sentLength);
	int recvfrom(char* buf, unsigned int bufSize, std::string* ip, unsigned int* port, unsigned int* receivedLength);
};

}// namespace OpenAPI

#endif//_OPENAPI_SOCKET_H_

