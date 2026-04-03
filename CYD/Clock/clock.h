//----------------------------------------------------------------------
// CYD Clock program
// =================
// This contains various constants and protoypes needed
//----------------------------------------------------------------------
#ifndef _INC_CLOCK
#define _INC_CLOCK

// Display
void InitDisplay();
void DisplayStatus(char *Format, ...);
void DisplayTime();
void DisplayTemp();
void SetBrightness(int Brightness);
bool IsTouched();

// Sensors
extern bool aht_init, bmp_init;
extern float Temperature, Humidity, Pressure;
void InitSensors();
void GetTemp();

// Non volatile storage
bool InitClockNVS();
bool SetSSID(const char* SSID);
const char* GetSSID();
bool SetPassword(const char* Password);
const char* GetPassword();

// Time
void UpdateTime(void* Unused);

// CLI
void ProcessSerial();

#endif // _INC_CLOCK