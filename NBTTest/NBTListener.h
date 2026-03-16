//----------------------------------------------------------------------
// NBTListener.h
// =============
// Code to listen for and respond to NBT name requests
//----------------------------------------------------------------------
#ifndef _INC_NBTLISTENER
#define _INC_NBTLISTENER

//----------------------------------------------------------------------
// Useful constants
//----------------------------------------------------------------------
// Maximum length of a Netbios name
#define LEN_NETBIOSNAME 15
// Maximum length of a v4 IP address aaa.bbb.ccc.ddd\0 = 16 chars
#define LEN_IPADDRESS 16

// Errors
#define NBT_ERR_OK        0 // success
#define NBT_ERR_INVALIDIP 1 // invalid IP address was supplied
#define NBT_ERR_CREATE    2 // cannot create the socket
#define NBT_ERR_BIND      3 // cannot bind the socket

//----------------------------------------------------------------------
// StartNBTListener
// ----------------
// Start listening for NBT name requests.
//----------------------------------------------------------------------
int StartNBTListener(const char* Name, const char* v4IPAddr);

#endif // _INC_NBTLISTENER