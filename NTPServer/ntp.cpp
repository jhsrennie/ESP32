// ---------------------------------------------------------------------
// ntp.cpp
// -------
// NTP code
// This is from Andreas Spiess's YouTube video:
// - https://www.youtube.com/watch?v=BGb2t5FT-zw
// His github repository for this is:
// - https://github.com/SensorsIot/NTP-Server-with-GPS/
// ---------------------------------------------------------------------
#include <WiFiUdp.h>

// Global UDP packet variable
bool udp_initialised = false;
WiFiUDP udp;

// ---------------------------------------------------------------------
// handleNTPRequest
// ----------------
// Respond to an NTP request
// ---------------------------------------------------------------------
void handleNTPRequest() {
  // Initialise the UDP connection if it hasn't already been done
  if (!udp_initialised) {
    udp.begin(123);
    udp_initialised = true;
  }

  // Wait for a connection
  int size = udp.parsePacket();
  // The size must be at least 48 bytes
  if (size < 48) {
    if (size > 0) // warn if we got a weird size
      Serial.printf("%s: Invalid packet size = %d\n", __func__, size);
    return;
  }

  // Process the request
  uint8_t req[48], resp[48];
  udp.read(req, 48);
  Serial.printf("%s: Got request\n", __func__);

  // 1) Decode the client’s Transmit Timestamp (T₁) from bytes 40..47
  uint32_t T1_sec  = (uint32_t)req[40]<<24 | (uint32_t)req[41]<<16 | (uint32_t)req[42]<<8 | (uint32_t)req[43];
  uint32_t T1_frac = (uint32_t)req[44]<<24 | (uint32_t)req[45]<<16 | (uint32_t)req[46]<<8 | (uint32_t)req[47];

  // 2) Record server receipt time (T₂)
  struct timeval tv2;
  gettimeofday(&tv2, NULL);
  uint32_t T2_sec  = tv2.tv_sec  + 2208988800UL;
  uint32_t T2_frac = (uint32_t)( (double)tv2.tv_usec * (4294967296.0/1e6) );

  // Build the response header (unchanged)…
  resp[0] = 0x24; resp[1] = 1; resp[2] = req[2]; resp[3] = (uint8_t)-6;
  memset(&resp[4], 0, 8);
  resp[12]='L'; resp[13]='O'; resp[14]='C'; resp[15]='L';

  // Reference Timestamp = T₂
  for (int i = 0; i < 4; i++) resp[16+i] = (T2_sec  >> (24-8*i)) & 0xFF;
  for (int i = 0; i < 4; i++) resp[20+i] = (T2_frac >> (24-8*i)) & 0xFF;

  // Originate Timestamp = client’s T₁
  memcpy(&resp[24], &req[40], 8);

  // 3) Record server transmit time (T₃) immediately before sending
  struct timeval tv3;
  gettimeofday(&tv3, NULL);
  uint32_t T3_sec  = tv3.tv_sec  + 2208988800UL;
  uint32_t T3_frac = (uint32_t)( (double)tv3.tv_usec * (4294967296.0/1e6) );
  for (int i = 0; i < 4; i++) resp[32+i] = (T3_sec  >> (24-8*i)) & 0xFF;
  for (int i = 0; i < 4; i++) resp[36+i] = (T3_frac >> (24-8*i)) & 0xFF;
  for (int i = 0; i < 4; i++) resp[40+i] = (T3_sec  >> (24-8*i)) & 0xFF;
  for (int i = 0; i < 4; i++) resp[44+i] = (T3_frac >> (24-8*i)) & 0xFF;

  // 4) Send the packet
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write(resp, 48);
  udp.endPacket();

  // 5) Log all three timestamps to Serial
  Serial.printf("%s: T1(client orig): %10u.%08u\n", __func__, T1_sec, T1_frac);
  Serial.printf("%s: T2(server recv): %10u.%08u\n", __func__, T2_sec, T2_frac);
  Serial.printf("%s: T3(server send): %10u.%08u\n", __func__, T3_sec, T3_frac);
  Serial.println("");
}