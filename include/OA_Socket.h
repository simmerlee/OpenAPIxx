#ifndef _OPENAPIXX_SOCKET_H_
#define _OPENAPIXX_SOCKET_H_

#include <string>
#include <stdint.h>

namespace OpenAPIxx
{

class Socket
{
public:
	Socket();

    // reserveFlag為true時
    // 析構時只釋放內存，不釋放socket所佔用的系統資源
    // 即socket_fd在析構以後依然有效
    Socket(const void* socketFd, bool reserveFlag);
	virtual ~Socket();
	virtual int create() = 0;
	int bind(uint32_t port);
	int connect(const std::string& ip, uint32_t port);
    int connect(const std::string& ip, uint32_t port, uint32_t timeout);
	int send(const char* buf, size_t length, size_t* sentLength);
	int recv(char* buf, size_t bufSize, size_t* receivedLength);
	int close();
    int setReserveFlag(bool flag);
	int setNoneBlockModel(bool flag);
	int setSendBufferSize(size_t size);
	int setRecvBufferSize(size_t size);
	int setSendTimeout(uint32_t timeoutMs);
	int setRecvTimeout(uint32_t timeoutMs);
    int getSocketName(uint32_t& port);
	const void* getSocketFd() const;
	int getLastError() const;
    static int GetHostByName(const std::string& domainName, 
                        std::string& ip);
    static uint16_t ntoh16(uint16_t num);
    static uint16_t hton16(uint16_t num);
    static uint32_t ntoh32(uint32_t num);
    static uint32_t hton32(uint32_t num);
    static uint64_t ntoh64(uint64_t num);
    static uint64_t hton64(uint64_t num);

protected:
    void* m_p;
};

class TCPSocket : public Socket
{
public:
	TCPSocket();
    TCPSocket(const void* socketFd, bool reserveFlag);
	int create();
	int listen(int backlog);
	int accept(TCPSocket*& clientSocket, std::string* ip, uint32_t* port);
};

class UDPSocket : public Socket
{
public:
	UDPSocket();
    UDPSocket(const void* socketFd, bool reserveFlag);
	int create();
	int sendto(const char* buf, size_t length,
                    const std::string& ip, uint32_t port, 
                    size_t *sentLength);
	int recvfrom(char* buf, size_t bufSize,
                    std::string* ip, uint32_t* port, 
                    size_t* receivedLength);
};

#define OA_SELECTOR_MAX_TIMEOUT             UINT32_MAX

class Selector {
public:
    Selector();
    ~Selector();
    void clearAllSet();
    void readSetSet(const OpenAPIxx::Socket* socket);
    bool readSetIsset(const OpenAPIxx::Socket* socket);
    void writeSetSet(const OpenAPIxx::Socket* socket);
    bool writeSetIsset(const OpenAPIxx::Socket* socket);
    void excpSetSet(const OpenAPIxx::Socket* socket);
    bool excpSetIsset(const OpenAPIxx::Socket* socket);

    // timeoutMs為0：立刻返回
    // timeoutMs為OA_SELECTOR_MAX_TIMEOUT，一直阻塞
    // timeoutMs為其他值，select最多等待對應的毫秒數
    // 成功返回0，否則返回非零
    int select(uint32_t timeoutMs, size_t* readyCount);
    int getLastError();
private:
    void* m_p;
};

}// namespace OpenAPIxx

typedef OpenAPIxx::Socket OASocket;
typedef OpenAPIxx::TCPSocket OATCPSocket;
typedef OpenAPIxx::UDPSocket OAUDPSocket;
typedef OpenAPIxx::Selector OASelector;

#endif//_OPENAPIXX_SOCKET_H_

