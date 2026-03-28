//----------------------------------------------------------------------
// CYD Clock program
// =================
// This contains various constants and protoypes needed
//----------------------------------------------------------------------
#ifndef _INC_CLOCK
#define _INC_CLOCK

// The maximum wi-fi SSID length is 31 bytes
#define MAX_LEN_SSID 31
// The maximum wi-fi password length is 63 bytes
#define MAX_LEN_PWD 63

// Function prototypes
bool InitClockNVS();
bool SetSSID(const char* SSID);
const char* GetSSID();
bool SetPassword(const char* Password);
const char* GetPassword();

void ProcessSerial();

#endif // _INC_CLOCK