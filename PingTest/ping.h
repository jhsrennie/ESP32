//----------------------------------------------------------------------
// ping.h
// ======
// Ping implementation for the ESP32 in C
//----------------------------------------------------------------------
#ifndef _INC_C_PING
#define _INC_C_PING

#ifdef __cplusplus
extern "C" {
#endif

// Error codes returned by the ping functions
#define PING_ERR_TIMEOUT  -1 // ping timed out
#define PING_ERR_HOSTNAME -2 // host name not found
#define PING_ERR_IPADDR   -3 // invalid IP address
#define PING_ERR_OTHER    -4 // all other errors

// Return the last error number
int ping_last_error();
// Return the last error string
const char* ping_last_error_msg();
// Ping a host name (requires DNS)
int ping_host(const char* hostname, int timeout);
// Ping an IP address passed as a string (does not require DNS)
int ping_ip(const char *ip_addr, int timeout);

#ifdef __cplusplus
}
#endif

#endif // _INC_C_PING