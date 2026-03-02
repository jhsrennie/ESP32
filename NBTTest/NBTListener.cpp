//----------------------------------------------------------------------
// NBTClient.cpp
// =============
// Code to listen for and respond to NBT name requests
//----------------------------------------------------------------------

// Uncomment this for extra debugging info
// #define NBT_DEBUGGING

#ifdef NBT_DEBUGGING
// We need the Arduino code only for Serial.print
#include <Arduino.h>
#endif

#include <stdint.h>
#include <string.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include "NBTListener.h"

#define NBNS_PORT 137
#define BUFFER_SIZE 1024

//----------------------------------------------------------------------
// Template for an RFC 1002 NBT Positive Name Query Response
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         NAME_TRN_ID           |1|  0x0  |1|T|1|?|0 0|0|  0x0  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          0x0000               |           0x0001              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          0x0000               |           0x0000              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// /                            RR_NAME                            /
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           NB (0x0020)         |         IN (0x0001)           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              TTL                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           RDLENGTH            |                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
// /                       ADDR_ENTRY ARRAY                        /
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//----------------------------------------------------------------------
#define LEN_REPLY 62
static unsigned char reply_template[] = {
  0x00, 0x00, // NAME_TRN_ID
  0x85, 0x00, // Flags
  0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00,

  0x20, // RR_NAME length
  0x00, 0x00, 0x00, 0x00, //RR_NAME encoded
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00,

  0x00, 0x20, // NB
  0x00, 0x01, // IN
  0x00, 0x04, 0x93, 0xe0, // TTL
  0x00, 0x06, // RDLENGTH
  0x40, 0x00, // NB_FLAGS. 0x40 = unique P node
  0x00, 0x00, 0x00, 0x00 // iP address in network byte ordrer
};


//----------------------------------------------------------------------
// struct used to pass the name and IP address to the task function
//----------------------------------------------------------------------
typedef struct _NBTDetails {
  // This is the Netbios name
  char name[LEN_NETBIOSNAME+1];
  // This is the v4 IP address
  char address[LEN_IPADDRESS+1];
  // Used to return an error code
  uint32_t nbterr;
} NBTDetails;

#ifdef NBT_DEBUGGING
//----------------------------------------------------------------------
// dump_packet
// -----------
// For debugging it's sometimes useful to dump a packet as hex
//----------------------------------------------------------------------
void dump_packet(unsigned char* data, int len) {
  for (int x = 0; x < len; x++) {
    Serial.printf("%02x ", data[x]);
    if (x % 4 == 3)
      Serial.printf("\n");
  }
  Serial.printf("\n");
}
#endif

//----------------------------------------------------------------------
// decode_netbios_name
// -------------------
// Netbios names are encoded in a silly way
//----------------------------------------------------------------------
static void decode_netbios_name(char *dest, const unsigned char *src) {
  for (int i = 0; i < 15; i++) {
    char high = (src[i * 2] - 'A') << 4;
    char low = (src[i * 2 + 1] - 'A');
    dest[i] = high | low;
  }
  dest[15] = '\0';
  for (int i = 14; i >= 0 && (dest[i] == ' ' || dest[i] == 0); i--)
    dest[i] = '\0';
}

//----------------------------------------------------------------------
// NBTListener
// -----------
// This is the task that runs the name resolution server
//----------------------------------------------------------------------
static void NBTListener(void* Parameter) {
  // The parameter is a pointer to an NBTDetails struct with the
  // Netbios name and associated IP address as strings.
  NBTDetails* details = (NBTDetails*) Parameter;
  details->nbterr = NBT_ERR_OK;

  // Copy the name
  char name[LEN_NETBIOSNAME+1];
  strlcpy(name, details->name, LEN_NETBIOSNAME+1);

  // Convert the IP address
  in_addr_t addr;
  if (0 == inet_aton(details->address, &addr)) {
    details->nbterr = NBT_ERR_INVALIDIP;
    vTaskDelete(NULL);
  }

  // Create the UDP socket that we'll use for listening for requests
  int s = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) {
    details->nbterr = NBT_ERR_CREATE;
    vTaskDelete(NULL);
  }
  // Allow address reuse to prevent "Address already in use" errors
  int opt = 1;
  lwip_setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // And listen on port 137
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = addr;
  server_addr.sin_port = htons(NBNS_PORT);
  if (0 != lwip_bind(s, (struct sockaddr*) &server_addr, sizeof(server_addr))) {
    lwip_close(s);
    details->nbterr = NBT_ERR_BIND;
    vTaskDelete(NULL);
  }

  // Now sit in a loop listening for requests
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    unsigned char nbtrequest[BUFFER_SIZE];
    int len = lwip_recvfrom(s, nbtrequest, sizeof(nbtrequest), 0, (struct sockaddr*) &client_addr, &addr_len);

#ifdef NBT_DEBUGGING
    // DEBUGGING: Dump the request packet
    Serial.printf("DEBUG: received %d bytes from %s\n", len, inet_ntoa(client_addr.sin_addr.s_addr));
    dump_packet(nbtrequest, len);
#endif

    // A valid name request must be at least 12 bytes
    if (len < 12) continue;

    // Only process queries. These have the top bit of the flags set to 0.
    uint16_t flags = (nbtrequest[2] << 8) | nbtrequest[3];
    if ((flags & 0x8000) != 0) continue;

    // The name field starts at index 13 (12-byte header + 1-byte length prefix)
    char requested_name[16];
    decode_netbios_name(requested_name, nbtrequest+13);
#ifdef NBT_DEBUGGING
    Serial.printf("DEBUG: name request for \"%s\"\n", requested_name);
#endif

    // Ignore the request if it's not for our name
    if (strcasecmp(requested_name, name) != 0)
      continue;

    // The request is for us so we need to construct the reply packet
    unsigned char nbtreply[LEN_REPLY];
    memcpy(nbtreply, reply_template, LEN_REPLY);

    // We need to copy the transaction id from the query
    nbtreply[0] = nbtrequest[0];
    nbtreply[1] = nbtrequest[1];

    // Copy the name from the request
    memcpy(nbtreply+12, nbtrequest+12, 34);

    // Copy in our IP address
    memcpy(nbtreply+58, &addr, 4);

    // Send the reply
    len = lwip_sendto(s, nbtreply, LEN_REPLY, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));

#ifdef NBT_DEBUGGING
    Serial.printf("DEBUG: reply sent %d bytes\n", len);
    dump_packet(nbtreply, LEN_REPLY);
#endif
  }
}

//----------------------------------------------------------------------
// StartNBTListener
// ----------------
// Function that starts the name query listener
// This is the only function the ESP32 sketch needs to call
//----------------------------------------------------------------------
int StartNBTListener(const char* Name, const char* v4IPAddr) {
  // Copy the details into a struct so we can pass them to the new task
  NBTDetails details;
  strlcpy(details.name, Name, LEN_NETBIOSNAME);
  strlcpy(details.address, v4IPAddr, LEN_IPADDRESS);

  // Create the task
  TaskHandle_t hNBTListener;
  xTaskCreate(
    NBTListener,
    "NBTListener",
    4000, // from experiment 4k seems enough
    (void*) &details,
    1,
    &hNBTListener
  );

  // Wait a moment to let the task start
  delay(1000);

  // Return the error code from the task
  return details.nbterr;
}