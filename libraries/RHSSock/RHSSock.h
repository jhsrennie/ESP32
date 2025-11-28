//**********************************************************************
// CRHSSocket
// ==========
//**********************************************************************

#ifndef _INC_CRHSSOCK
#define _INC_CRHSSOCK

#include <lwip/sockets.h>
#include <lwip/netdb.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define TIMEOUT_INFINITE -1

//**********************************************************************
// CRHSSocket
// ----------
//**********************************************************************

class CRHSSocket
{

  public:
    CRHSSocket();
    ~CRHSSocket();

    bool Connect(const char* Peer, int PeerPort, const char* Host = NULL, int HostPort = 0);

    bool Listen(char* Host, int Port);
    CRHSSocket* Accept(char* Peer, int MaxLen);

    int printf(const char* Format, ...);

    char* gets(char* Data, int MaxLen);
    bool  puts(const char* Data);

    char sgetc(void);
    char sputc(char c);

    int read(void* Data, size_t Size, size_t NumObj);
    int write(const void* Data, size_t Size, size_t NumObj);

    int recv(char* buf, int len, int flags);
    int send(const char* buf, int len, int flags);
    int recvfrom(char* buf, int len, int flags);
    int sendto(const char* buf, int len, int flags);

    int select(bool Read = true, bool Write = false, bool Error = false, uint32_t Timeout = 0);

    int ReadTimeout(char* buf, int len, int flags);
    int WriteTimeout(const char* buf, int len, int flags);

    inline void Timeout(uint32_t t) { m_Timeout = t; }
    inline uint32_t Timeout(void) { return(m_Timeout); }

    bool Shutdown(int How);
    bool CloseSocket(void);
    bool Bind(const char* Host, int Port);
    inline bool BindSocket(char* Host, int Port) { return Bind(Host, Port); }
    bool BindInRange(char* Host, int Low, int High, int Retries);
    bool Linger(bool LingerOn, int LingerTime = 0);
    bool CreateSocket(int Type = SOCK_STREAM, int Protocol = IPPROTO_TCP);

    bool Socket(int sockfd, bool Bound = true);
    inline int Socket(void) { return(m_Socket); }

    bool SetPeer(const char* Peer, int Port);
    bool GetPeer(char* Peer, int MaxLen, int* Port);
    bool SetHost(const char* Host, int Port);
    bool GetHost(char* Host, int MaxLen, int* Port);
    // The timeout is in seconds
    inline void SetTimeout(int Timeout) { m_Timeout = Timeout; };
    inline int GetTimeout(void) { return m_Timeout; };

    uint32_t HostToIP(const char* Hostname);

    const char* GetLastErrorMsg(void);
    inline int GetLastError(void) { return(m_LastError); }

  private:
    int m_Socket;
    bool m_Bound;
    struct sockaddr m_HostAddr;
    struct sockaddr m_PeerAddr;

    int m_Timeout;

    int m_LastError;
};


//**********************************************************************
// End of CRHSSocket
// -----------------
//**********************************************************************

#endif // _INC_CRHSSOCK