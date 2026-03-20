//----------------------------------------------------------------------
// mDNSListener.h
// ==============
// Code to listen for and respond to mDNS name requests
//----------------------------------------------------------------------
#ifndef _INC_MDNSLISTENER
#define _INC_MDNSLISTENER

//----------------------------------------------------------------------
// Useful constants
//----------------------------------------------------------------------
// Maximum length of a hostname
#define LEN_HOSTNAME 253
// Maximum length of a v4 IP address aaa.bbb.ccc.ddd = 16 chars
#define LEN_IPADDRESS 15

// Errors
#define MDNS_ERR_OK        0 // success
#define MDNS_ERR_TOOLONG   1 // invalid IP address was supplied
#define MDNS_ERR_INVALIDIP 2 // invalid IP address was supplied
#define MDNS_ERR_CREATE    3 // cannot create the socket
#define MDNS_ERR_BIND      4 // cannot bind the socket
#define MDNS_ERR_ADD       5 // cannot add to the multicast group

//----------------------------------------------------------------------
// StartmDNSListener
// ----------------
// Start listening for mDNS name requests.
//----------------------------------------------------------------------
int StartmDNSListener(const char* Name, const char* v4IPAddr);

#endif // _INC_MDNSLISTENER