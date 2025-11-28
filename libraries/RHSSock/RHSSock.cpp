//**********************************************************************
// CRHSSocket
// ==========
//**********************************************************************

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include "RHSSock.h"

//**********************************************************************
// Constructor & Destructor
// ------------------------
//**********************************************************************

CRHSSocket::CRHSSocket() {
  m_Socket = INVALID_SOCKET;

  memset(&m_HostAddr, 0, sizeof(m_HostAddr));
  memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));

  ((struct sockaddr_in*) &m_HostAddr)->sin_family = AF_INET;
  ((struct sockaddr_in*) &m_PeerAddr)->sin_family = AF_INET;

  m_Bound = false;
  m_Timeout = TIMEOUT_INFINITE;
}

CRHSSocket::~CRHSSocket() {
  // If we have created a socket close it now
  if (m_Socket != INVALID_SOCKET)
    closesocket(m_Socket);
}


//**********************************************************************
// CRHSSocket::Connect
// -------------------
// Connect to a server
// This method will do all initialisation and binding if necessary, so
// usually just this one call is all that is needed to connect a socket.
//**********************************************************************

bool CRHSSocket::Connect(const char* Peer, int PeerPort, const char* Host, int HostPort) {
  // If we aren't already bound set the host and bind
  if (!Bind(Host, HostPort))
    if (m_LastError != 0)
      return false;

  // Set the peer
  if (!SetPeer(Peer, PeerPort))
    return false;

  // Connect
  // For some reason connect fails if we pass the class member variable
  // m_PeerAddr but works fine if we use a local variable.
  struct sockaddr peeraddr;
  memcpy(&peeraddr, &m_PeerAddr, sizeof(struct sockaddr));

  if (connect(m_Socket, &peeraddr, sizeof(struct sockaddr)) != 0)
  { m_LastError = errno;
    return false;
  }
  memcpy(&m_PeerAddr, &peeraddr, sizeof(struct sockaddr));

  // Return indicating success
  return true;
}


//**********************************************************************
// CRHSSocket::Listen
// ------------------
// Start listening on a socket
//**********************************************************************

bool CRHSSocket::Listen(char* Host, int Port)
{

// If we aren't already bound set the host and bind

  if (!Bind(Host, Port))
    if (m_LastError != 0)
      return false;

// Listen on the port

  if (listen(m_Socket, 5) != 0)
  { m_LastError = errno;
    return false;
  }

// Return indicating success

  return true;
}


//**********************************************************************
// CRHSSocket::Accept
// ------------------
// Wait for a client connection
// This method blocks until a connection is made or an error occurs.
//**********************************************************************

CRHSSocket* CRHSSocket::Accept(char* Peer, int MaxLen)
{
  CRHSSocket* accepted;
  char* peer;

// Allocate a new object to hold the accepted socket

  accepted = new CRHSSocket;
  if (!accepted)
  { m_LastError = errno;
    return(NULL);
  }

// Accept a connection

  socklen_t len = sizeof(struct sockaddr);
  accepted->m_Socket = accept(m_Socket, &accepted->m_PeerAddr, &len);

  if (accepted->m_Socket == INVALID_SOCKET)
  { m_LastError = errno;
    delete accepted;
    return(NULL);
  }

// We've got a connection so fill in the other bits of the new object

  accepted->m_HostAddr = m_HostAddr;
  accepted->m_Bound = true;

// If required get the name of the connected client

  if (Peer)
  { peer = inet_ntoa(((struct sockaddr_in*) &accepted->m_PeerAddr)->sin_addr);
    if (peer)
    { strncpy(Peer, peer, MaxLen);
      Peer[MaxLen] = '\0';
    }
    else
    { strcpy(Peer, "");
    }
  }

// Return the new object

  return(accepted);
}


//**********************************************************************
// CRHSSocket::printf
// ------------------
// Just like the ANSI C printf except that it uses the Win32 sprintf
// which is more limited.  The main restriction is no floating point
// support.
//**********************************************************************

int CRHSSocket::printf(const char* Format, ...)
{ int i;
  va_list ap;
  char s[0x1000];

  va_start(ap, Format);
  vsnprintf(s, 0x0FFF, Format, ap);
  va_end(ap);

  i = WriteTimeout(s, strlen(s), 0);

  return(i);
}


//**********************************************************************
// CRHSSocket::gets
// ----------------
// Reads a string terminated by \n.  NB the terminating \n or \r\n is
// stripped.
// MaxLen is the maximum length of the string so the array needs to have
// a length of at least MaxLen+1.
// If a non-NULL value is returned then at least one character was read.
// The test for no data read or an error is a return value of NULL.
//
// CRHSSocket::puts
// ----------------
// Returns true if at least one character was written or false if no
// characters were written or an error occurred.
//**********************************************************************

char* CRHSSocket::gets(char* Data, int MaxLen)
{ int i, j;

  for (i = 0; i < MaxLen; i++)
  { j = ReadTimeout(Data + i, 1, 0);
    if (j == SOCKET_ERROR || j == 0)
    { Data[i] = '\0';
      return(i > 0 ? Data : NULL);
    }

    if (Data[i] == '\n')
      break;
  }

  Data[i] = '\0';

  if (i > 0)
    if (Data[i-1] == '\r')
      Data[i-1] = '\0';

  return(Data);
}


bool CRHSSocket::puts(const char* Data)
{ int tosend, sent, retries, i;

  tosend = strlen(Data);
  sent = retries = 0;

  while (tosend > 0)
  { i = WriteTimeout(Data + sent, tosend, 0);
    if (i == SOCKET_ERROR || i == 0)
      return(sent > 0 ? true : false);

    sent += i;
    tosend -= i;

    if (++retries > 5)
    { m_LastError = 0;
      return false;
    }
  }

  return true;
}


//**********************************************************************
// CRHSSocket::sgetc
// CRHSSocket::sputc
// -----------------
// Just like getc and putc.  The reason for not calling the methods getc
// and putc is to avoid the preprocessor interpreting them as macros.
//**********************************************************************

char CRHSSocket::sgetc(void)
{ int i;
  char c;

  i = ReadTimeout(&c, 1, 0);
  if (i == SOCKET_ERROR || i == 0)
    return(0);

  return(c);
}


char CRHSSocket::sputc(char c)
{ int i;

  i = WriteTimeout(&c, 1, 0);
  if (i == SOCKET_ERROR || i == 0)
    return(0);

  return(c);
}


//**********************************************************************
// CRHSSocket::read
// CRHSSocket::write
// -----------------
// Like the C read and write functions
//**********************************************************************

int CRHSSocket::read(void* Data, size_t Size, size_t NumObj)
{ int i;

  i = ReadTimeout((char*) Data, Size*NumObj, 0);

  return(i);
}


int CRHSSocket::write(const void* Data, size_t Size, size_t NumObj)
{ int i;

  i = WriteTimeout((char*) Data, Size*NumObj, 0);

  return(i);
}


//**********************************************************************
// CRHSSocket::recv
// CRHSSocket::send
// ----------------
// Standard sockets recv and send
//**********************************************************************

int CRHSSocket::recv(char* buf, int len, int flags) {
  int i = ::recv(m_Socket, buf, len, flags);
  if (i == SOCKET_ERROR) 
    m_LastError = errno;

  // If the function reports success but no data was read this means the
  // connection has been closed
  else if (i == 0)
    m_LastError = ENOTCONN;

  return i;
}


int CRHSSocket::send(const char* buf, int len, int flags) {
  int i = ::send(m_Socket, buf, len, flags);
  if (i == SOCKET_ERROR)
    m_LastError = errno;

  return i;
}


int CRHSSocket::recvfrom(char* buf, int len, int flags) {
  socklen_t peerlen = sizeof(m_PeerAddr);
  int i = ::recvfrom(m_Socket, buf, len, flags, &m_PeerAddr, &peerlen);
  if (i == SOCKET_ERROR)
    m_LastError = errno;

  return i;
}


int CRHSSocket::sendto(const char* buf, int len, int flags) {
  int i = ::sendto(m_Socket, buf, len, flags, &m_PeerAddr, sizeof(struct sockaddr));
  if (i == SOCKET_ERROR)
    m_LastError = errno;

  return i;
}


//**********************************************************************
// CRHSSocket::select
// ------------------
// This is a simplified form of the sockets select function.
//
// Args:
//   Read    - check for data waiting to be read (default true)
//   Write   - check for data waiting to be written (default false)
//   Error   - check for errors (default false)
//   Timeout - timout for select in milliseconds (default 0)
//
// Just calling select() checks for data waiting in the input buffer.
// Supply arguments for other checks.
//**********************************************************************

int CRHSSocket::select(bool Read, bool Write, bool Error, uint32_t Timeout) {
  int select_result;
  fd_set fd_read, *pfd_read = NULL;
  fd_set fd_write, *pfd_write = NULL;
  fd_set fd_error, *pfd_error = NULL;
  struct timeval timeout;

  // Check at least one select has been asked for
  if (!Read && !Write && !Error)
    return 0;

  // Initialise the fd_sets and the timeout
  if (Read) {
    pfd_read = &fd_read;
    FD_ZERO(pfd_read);
    FD_SET(m_Socket, pfd_read);
  }
    
  if (Write) {
    pfd_write = &fd_write;
    FD_ZERO(pfd_write);
    FD_SET(m_Socket, pfd_write);
  }
    
  if (Error) {
    pfd_error = &fd_error;
    FD_ZERO(pfd_error);
    FD_SET(m_Socket, pfd_error);
  }

  timeout.tv_sec = Timeout/1000;
  timeout.tv_usec = (Timeout - (timeout.tv_sec*1000))*1000;

  // Do the select
  select_result = ::select(0, pfd_read, pfd_write, pfd_error, &timeout);
  if (select_result == 0)
    m_LastError = ETIMEDOUT;

  // All done
  return select_result;
}


//**********************************************************************
// CRHSSocket::ReadTimeout
// -----------------------
// This returns 0 and sets the last error to EWOULDBLOCK if there is a
// timeout
//**********************************************************************

int CRHSSocket::ReadTimeout(char* buf, int len, int flags) {
  // If there is a timeout use a select to see if we can read in that time
  if (m_Timeout != TIMEOUT_INFINITE) {
    // Create the read descriptor
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_Socket, &rfds);

    // The timeout setting m_Timeout is in seconds
    struct timeval tv;
    tv.tv_sec = m_Timeout;
    tv.tv_usec = 0;

    // Do the select
    int sel = ::select(1, &rfds, NULL, NULL, &tv);
    // Check for errors
    if (sel == SOCKET_ERROR) {
      m_LastError = errno;
      return SOCKET_ERROR;
    }

    // If there was a timeout return now
    if (sel == 0) {
      m_LastError = EWOULDBLOCK;
      return 0;
    }
  }

  // Do the read
  int i = ::recv(m_Socket, buf, len, flags);
  if (i == SOCKET_ERROR)
    m_LastError = errno;

  // If the function reports success but no data was read this means the
  // connection has been closed
  if (i == 0)
    m_LastError = ENOTCONN;

  // All done
  return i;
}


//**********************************************************************
// CRHSSocket::WriteTimeout
// ------------------------
// Obvious partner to ReadTimeout.
//**********************************************************************

int CRHSSocket::WriteTimeout(const char* buf, int len, int flags) {
  // If there is a timeout use a select to see if we can write in that time
  if (m_Timeout != TIMEOUT_INFINITE) {
    // Create the write descriptor
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(m_Socket, &wfds);

    // The timeout setting m_Timeout is in seconds
    struct timeval tv;
    tv.tv_sec = m_Timeout;
    tv.tv_usec = 0;

    // Do the select
    int sel = ::select(2, NULL, &wfds, NULL, &tv);
    // Check for errors
    if (sel == SOCKET_ERROR) {
      m_LastError = errno;
      return SOCKET_ERROR;
    }

    // If there was a timeout return now
    if (sel == 0) {
      m_LastError = EWOULDBLOCK;
      return 0;
    }
  }

  // Do the write
  int i = ::send(m_Socket, buf, len, flags);
  if (i == SOCKET_ERROR)
    m_LastError = errno;

  // All done
  return i;
}


//**********************************************************************
// CRHSSocket::Shutdown
// --------------------
// Like the sockets shutdown function
//**********************************************************************

bool CRHSSocket::Shutdown(int How)
{
  if (shutdown(m_Socket, How) != 0)
  { m_LastError = errno;
    return false;
  }

  return true;
}


//**********************************************************************
// CRHSSocket::CloseSocket
// -----------------------
// This closes the socket and resets all internal variables.
// The object is now ready for reuse.
//**********************************************************************

bool CRHSSocket::CloseSocket(void)
{

// Close the socket

  if (closesocket(m_Socket) != 0)
  { m_LastError = errno;
    return false;
  }

// Reset all internal variables to defaults

  m_Socket = INVALID_SOCKET;

  memset(&m_HostAddr, 0, sizeof(m_HostAddr));
  memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));

  ((struct sockaddr_in*) &m_HostAddr)->sin_family = AF_INET;
  ((struct sockaddr_in*) &m_PeerAddr)->sin_family = AF_INET;

  m_Bound = false;

  m_Timeout = TIMEOUT_INFINITE;

// All done

  return true;
}


//**********************************************************************
// CRHSSocket::Bind
// ----------------
// Like the sockets bind function
//**********************************************************************

bool CRHSSocket::Bind(const char* Host, int Port) {
// If we are already bound return false but with no last error

  m_LastError = 0;
  if (m_Bound)
    return false;

  // If the socket hasn't been created create it now
  if (m_Socket == INVALID_SOCKET)
    if (!CreateSocket())
      return false;

  // If a host name was supplied then use it
  if (!SetHost(Host, Port))
    return false;

  // Now bind the socket
  // For some reason connect fails if we pass the class member variable
  // m_HostAddr but works fine if we use a local variable.
  struct sockaddr hostaddr;
  memcpy(&hostaddr, &m_HostAddr, sizeof(struct sockaddr));

  if (bind(m_Socket, &hostaddr, sizeof(struct sockaddr)) != 0)
  { m_LastError = errno;
    return false;
  }
  memcpy(&m_HostAddr, &hostaddr, sizeof(struct sockaddr));

  m_Bound = true;

// Return indicating success
  return true;
}


//**********************************************************************
// CRHSSocket::BindInRange
// -----------------------
// Bind a socket from a range of ports
//**********************************************************************

bool CRHSSocket::BindInRange(char* Host, int Low, int High, int Retries)
{ int port, i;

// If we are already bound return false but with no last error

  m_LastError = 0;

  if (m_Bound)
    return false;

// If the socket hasn't been created create it now

  if (m_Socket == INVALID_SOCKET)
    if (!CreateSocket())
      return false;

// If the number of retries is greater than the range then work through the
// range sequentially.

  if (Retries > High - Low)
  { for (port = Low; port <= High; port++)
    { if (!SetHost(Host, port))
        return false;

      if (bind(m_Socket, &m_HostAddr, sizeof(m_HostAddr)) == 0)
        break;
    }

    if (port > High)
    { m_LastError = errno;
      return false;
    }
  }

// Else select numbers within the range at random

  else
  { for (i = 0; i < Retries; i++)
    { port = Low + (rand() % (High - Low));

      if (!SetHost(Host, port))
        return false;

      if (bind(m_Socket, &m_HostAddr, sizeof(m_HostAddr)) == 0)
        break;
    }

    if (i >= Retries)
    { m_LastError = errno;
      return false;
    }
  }

// Return indicating success

  m_Bound = true;

  return true;
}


//**********************************************************************
// CRHSSocket::Linger
// ------------------
// Set the linger option
//**********************************************************************

bool CRHSSocket::Linger(bool LingerOn, int LingerTime)
{ int i;
  bool b;
  struct linger l;

// If the socket hasn't been created create it now

  if (m_Socket == INVALID_SOCKET)
    if (!CreateSocket())
      return false;

// Set the lingering

  if (LingerOn)
  { l.l_onoff = 1;
    l.l_linger = LingerTime;
    i = setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (const char*) &l, sizeof(l));
  }
  else
  { b = 1; // true turns linger off
    i = setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (const char*) &b, sizeof(b));
  }

// Check for errors

  if (i != 0)
  { m_LastError = errno;
    return false;
  }

// Return indicating success

  return true;
}


//**********************************************************************
// CRHSSocket::CreateSocket
// ------------------------
// Create the socket
// You almost never need to call this directly as other methods like
// Connect call it when required.
//**********************************************************************

bool CRHSSocket::CreateSocket(int Type, int Protocol) {
  // If the socket has already been created just return true
  if (m_Socket != INVALID_SOCKET)
    return true;

  // Create the socket
  m_Socket = socket(AF_INET, Type, Protocol);
  if (m_Socket == INVALID_SOCKET) {
    m_LastError = errno;
    return false;
  }

// By default set no lingering
// I suspect this may be causing problems
//  Linger(false, 0);

  // Return indicating success
  return true;
}


//**********************************************************************
// CRHSSocket::Socket
// ------------------
// Attach to an existing socket
// NB there is no checking that the socket is valid
//**********************************************************************

bool CRHSSocket::Socket(int sockfd, bool Bound)
{
  if (sockfd == INVALID_SOCKET)
    return false;

  if (m_Socket != INVALID_SOCKET)
    CloseSocket();

  m_Socket  = sockfd;
  m_Bound   = Bound;
  m_Timeout = TIMEOUT_INFINITE;

  socklen_t len = sizeof(m_HostAddr);
  getsockname(m_Socket, &m_HostAddr, &len);
  len = sizeof(m_PeerAddr);
  getpeername(m_Socket, &m_PeerAddr, &len);

  return true;
}


//**********************************************************************
// CRHSSocket::SetPeer
// CRHSSocket::GetPeer
// -------------------
// Set and get the peer
//**********************************************************************

bool CRHSSocket::SetPeer(const char* Peer, int Port)
{
  // Clear the current peer settings
  memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));

  struct sockaddr_in* p_addr_in = (struct sockaddr_in*) &m_PeerAddr;
  p_addr_in->sin_family = AF_INET;
  if (Port > 0)
    p_addr_in->sin_port = htons(Port);

  // See if the peer is an IP address
  uint32_t peeraddress = inet_addr(Peer);
  if (peeraddress == INADDR_NONE) {
    // If the peer is not an IP address try resolving the name
    peeraddress = HostToIP(Peer);
    // If we still couldn't get the IP address give up
    if (peeraddress == INADDR_NONE) {
      return false;
    }
  }
  p_addr_in->sin_addr.s_addr = peeraddress;

// Return indicating success

  return true;
}


bool CRHSSocket::GetPeer(char* Peer, int MaxLen, int* Port)
{ char* peer;

  if (Peer)
  { peer = inet_ntoa(((struct sockaddr_in*) &m_PeerAddr)->sin_addr);
    if (!peer)
    { m_LastError = errno;
      return false;
    }

    strncpy(Peer, peer, MaxLen);
    Peer[MaxLen] = '\0';
  }

  if (Port)
    *Port = ntohs(((struct sockaddr_in*) &m_PeerAddr)->sin_port);

  return true;
}


//**********************************************************************
// CRHSSocket::SetHost
// CRHSSocket::GetHost
// -------------------
// Set and get the host
//**********************************************************************

bool CRHSSocket::SetHost(const char* Host, int Port) {
  // Initialise the current host settings
  memset(&m_HostAddr, 0, sizeof(m_HostAddr));
  struct sockaddr_in* p_addr_in = (struct sockaddr_in*) &m_HostAddr;
  p_addr_in->sin_family = AF_INET;
  if (Port > 0)
    p_addr_in->sin_port = htons(Port);

  // If we were passed a host name set it now
  if (Host) {
    // See if the host is an IP address
    uint32_t hostaddress = inet_addr(Host);
    if (hostaddress == INADDR_NONE) {
      // If the peer is not an IP address try resolving the name
      hostaddress = HostToIP(Host);
      // If we still couldn't get the IP address give up
      if (hostaddress == INADDR_NONE) {
        return false;
      }
    }
    p_addr_in->sin_addr.s_addr = hostaddress;
  }

  // Return indicating success
  return true;
}


bool CRHSSocket::GetHost(char* Host, int MaxLen, int* Port)
{ char* host;

  if (Host)
  { host = inet_ntoa(((struct sockaddr_in*) &m_HostAddr)->sin_addr);
    if (!host)
    { m_LastError = errno;
      return false;
    }

    strncpy(Host, host, MaxLen);
    Host[MaxLen] = '\0';
  }

  if (Port)
    *Port = ntohs(((struct sockaddr_in*) &m_HostAddr)->sin_port);

  return true;
}


//**********************************************************************
// CRHSSocket::HostToIP
// --------------------
// Get the IP address as a uint32_t from the host name
// #define	EAI_ADDRFAMILY 1 /* address family for host not supported */
// #define	EAI_AGAIN      2 /* temporary failure in name resolution */
// #define	EAI_BADFLAGS   3 /* invalid value for ai_flags */
// #define	EAI_FAIL       4 /* non-recoverable failure in name resolution */
// #define	EAI_FAMILY     5 /* ai_family not supported */
// #define	EAI_MEMORY     6 /* memory allocation failure */
// #define	EAI_NODATA     7 /* no address associated with host */
// #define	EAI_NONAME     8 /* host nor service provided, or not known */
// #define	EAI_SERVICE    9 /* service not supported for ai_socktype */
// #define	EAI_SOCKTYPE  10 /* ai_socktype not supported */
// #define	EAI_SYSTEM    11 /* system error returned in errno */
//**********************************************************************

uint32_t CRHSSocket::HostToIP(const char* Hostname) {
  uint32_t ipaddr = 0;

  // The only hint we need is that it's IPv4
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;

  // Get the info
  struct addrinfo* result;
  int s = getaddrinfo(Hostname, NULL, &hints, &result);
  if (s != 0) {
    m_LastError = s;
    return 0;
  }

  // Save the first address and free the result
  ipaddr = ((struct sockaddr_in*) result->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(result);

  // Return the address
  return ipaddr;
}


//**********************************************************************
// CRHSSocket::GetLastErrorMsg
// ---------------------------
//**********************************************************************

const char* CRHSSocket::GetLastErrorMsg(void) {
  static char errmsg[256];

  snprintf(errmsg, 255, "Unknown error: %d", m_LastError);
  char* s = errmsg;

  switch (m_LastError) {
    case EPERM:
      s = "Operation not permitted";
      break;

    case ENOENT:
      s = "No such file or directory";
      break;

    case ESRCH:
      s = "No such process";
      break;

    case EINTR:
      s = "Interrupted system call";
      break;

    case EIO:
      s = "I/O error";
      break;

    case ENXIO:
      s = "No such device or address";
      break;

    case E2BIG:
      s = "Arg list too long";
      break;

    case ENOEXEC:
      s = "Exec format error";
      break;

    case EBADF:
      s = "Bad file number";
      break;

    case ECHILD:
      s = "No child processes";
      break;

    case ENOMEM:
      s = "Out of memory";
      break;

    case EACCES:
      s = "Permission denied";
      break;

    case EFAULT:
      s = "Bad address";
      break;

    case EBUSY:
      s = "Device or resource busy";
      break;

    case EEXIST:
      s = "File exists";
      break;

    case EXDEV:
      s = "Cross-device link";
      break;

    case ENODEV:
      s = "No such device";
      break;

    case ENOTDIR:
      s = "Not a directory";
      break;

    case EISDIR:
      s = "Is a directory";
      break;

    case EINVAL:
      s = "Invalid argument";
      break;

    case ENFILE:
      s = "File table overflow";
      break;

    case EMFILE:
      s = "Too many open files";
      break;

    case ENOTTY:
      s = "Not a typewriter";
      break;

    case ETXTBSY:
      s = "Text file busy";
      break;

    case EFBIG:
      s = "File too large";
      break;

    case ENOSPC:
      s = "No space left on device";
      break;

    case ESPIPE:
      s = "Illegal seek";
      break;

    case EROFS:
      s = "Read-only file system";
      break;

    case EMLINK:
      s = "Too many links";
      break;

    case EPIPE:
      s = "Broken pipe";
      break;

    case EDOM:
      s = "Math argument out of domain of func";
      break;

    case ERANGE:
      s = "Math result not representable";
      break;

    case EDEADLK:
      s = "Resource deadlock would occur";
      break;

    case ENAMETOOLONG:
      s = "File name too long";
      break;

    case ENOLCK:
      s = "No record locks available";
      break;

    case ENOSYS:
      s = "Function not implemented";
      break;

    case ENOTEMPTY:
      s = "Directory not empty";
      break;

    case ELOOP:
      s = "Too many symbolic links encountered";
      break;

    case EWOULDBLOCK:
      s = "Operation would block";
      break;

    case ENOMSG:
      s = "No message of desired type";
      break;

    case EIDRM:
      s = "Identifier removed";
      break;

    case ENODATA:
      s = "No data available";
      break;

    case ETIME:
      s = "Timer expired";
      break;

    case ENOSR:
      s = "Out of streams resources";
      break;

    case ENOLINK:
      s = "Link has been severed";
      break;

    case EPROTO:
      s = "Protocol error";
      break;

    case EMULTIHOP:
      s = "Multihop attempted";
      break;

    case EBADMSG:
      s = "Not a data message";
      break;

    case EOVERFLOW:
      s = "Value too large for defined data type";
      break;

    case EILSEQ:
      s = "Illegal byte sequence";
      break;

    case ENOTSOCK:
      s = "Socket operation on non-socket";
      break;

    case EDESTADDRREQ:
      s = "Destination address required";
      break;

    case EMSGSIZE:
      s = "Message too long";
      break;

    case EPROTOTYPE:
      s = "Protocol wrong type for socket";
      break;

    case ENOPROTOOPT:
      s = "Protocol not available";
      break;

    case EPROTONOSUPPORT:
      s = "Protocol not supported";
      break;

    case EPFNOSUPPORT:
      s = "Protocol family not supported";
      break;

    case EAFNOSUPPORT:
      s = "Address family not supported by protocol";
      break;

    case EADDRINUSE:
      s = "Address already in use";
      break;

    case EADDRNOTAVAIL:
      s = "Cannot assign requested address";
      break;

    case ENETDOWN:
      s = "Network is down";
      break;

    case ENETUNREACH:
      s = "Network is unreachable";
      break;

    case ENETRESET:
      s = "Network dropped connection because of reset";
      break;

    case ECONNABORTED:
      s = "Software caused connection abort";
      break;

    case ECONNRESET:
      s = "Connection reset by peer";
      break;

    case ENOBUFS:
      s = "No buffer space available";
      break;

    case EISCONN:
      s = "Transport endpoint is already connected";
      break;

    case ENOTCONN:
      s = "Transport endpoint is not connected";
      break;

    case ESHUTDOWN:
      s = "Cannot send after transport endpoint shutdown";
      break;

    case ETOOMANYREFS:
      s = "Too many references: cannot splice";
      break;

    case ETIMEDOUT:
      s = "Connection timed out";
      break;

    case ECONNREFUSED:
      s = "Connection refused";
      break;

    case EHOSTDOWN:
      s = "Host is down";
      break;

    case EHOSTUNREACH:
      s = "No route to host";
      break;

    case EALREADY:
      s = "Operation already in progress";
      break;

    case EINPROGRESS:
      s = "Operation now in progress";
      break;

    case ESTALE:
      s = "Stale NFS file handle";
      break;

    case EDQUOT:
      s = "Quota exceeded";
      break;
  }

  return s;
}

//**********************************************************************
// End of rhssock.cpp
//**********************************************************************

