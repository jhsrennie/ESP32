//----------------------------------------------------------------------
// LWIPPing.h
// ==========
// Class to ping a host using the LWIP API
//----------------------------------------------------------------------
#ifndef _INC_LWIPPING
#define _INC_LWIPPING

// Error codes returned by the ping functions
#define LWIPPING_ERR_TIMEOUT  -1 // ping timed out
#define LWIPPING_ERR_HOSTNAME -2 // host name not found
#define LWIPPING_ERR_IPADDR   -3 // invalid IP address
#define LWIPPING_ERR_OTHER    -4 // all other errors

class LWIPPing {
  public:
    LWIPPing();

    // Ping a host by name - requires DNS
    int ping_host(const char* hostname, int timeout = 1000);
    // Ping a host by IP address - works when no DNS is available
    int ping_ip(const char* ip_addr, int timeout = 1000);

    int last_error() const { return errnum; }
    const char* last_error_msg() const { return errmsg; };

  private:
    // Last error message
    #define MAX_ERRMSG 256
    int errnum;
    char errmsg[MAX_ERRMSG];
};

#endif // _INC_LWIPPING