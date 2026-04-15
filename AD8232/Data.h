//----------------------------------------------------------------------
// Data.h
// ======
// Data collection module.
//----------------------------------------------------------------------
#ifndef _INC_DATA
#define _INC_DATA

// Size of data buffer
#define ADC_INTERVAL  10 // 10ms between data readings = 10 second sample
#define MAX_ADCDATA 1000 // Size of data buffer

// Prototypes
bool DataStart();
int DataRead(uint16_t data[MAX_ADCDATA]);

#endif // _INC_DATA