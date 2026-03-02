//----------------------------------------------------------------------
// nbtlookup
// ---------
// App to perform Netbios name lookups.
//----------------------------------------------------------------------
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <ctype.h>

// Uncomment this for extra debugging info
// #define NBT_DEBUGGING

// We need to link the winsock library
#pragma comment(lib, "ws2_32.lib")

// Syntax
#define SYNTAX "Syntax: nbtlookup <name>\n"

#ifdef NBT_DEBUGGING
//----------------------------------------------------------------------
// dump_packet
// -----------
// For debugging it's useful to dump a packet as hex
//----------------------------------------------------------------------
void dump_packet(unsigned char* data, int len) {
  for (int x = 0; x < len; x++) {
    printf("%02x ", data[x]);
    if (x % 4 == 3)
      printf("\n");
  }
  printf("\n");
}
#endif

//----------------------------------------------------------------------
// encode_netbios_name
// -------------------
// NetBIOS names are 16 chars padded with spaces. The last, 16th, char
// is the 'type' byte.
// The name is encoded in an odd way. See RFC1002 for the gory details.
//----------------------------------------------------------------------
void encode_netbios_name(char *dest, const char *src) {
  // Process only the first 15 characters as the 16th is the type byte
  for (int i = 0; i < 15; i++) {
    char c = (i < strlen(src)) ? src[i] : ' ';
    // Netbios names must be in uppercase
    if (c >= 'a' && c <= 'z')
      c = toupper(c);
    // Encode this character
    dest[i*2]     = ((c >> 4) & 0x0F) + 'A';
    dest[i*2 + 1] = (c & 0x0F) + 'A';
  }

  // 16th byte: 0x00 = Workstation Service
  dest[30] = 'A';
  dest[31] = 'A';
}

//----------------------------------------------------------------------
// main
// ----
//----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  char recv_buf[1024];

  // Check the arguments
  if (argc != 2) {
    printf(SYNTAX);
    return 1;
  }

  if (strcmp(argv[1], "-?") == 0) {
    printf(SYNTAX);
    return 0;
  }

  printf("Sending Netbios name request for %s\n", argv[1]);

  // initialise winsock
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    printf("Error initialising winsock\n");
    return 1;
  }

  // This is the query packet with the name field left blank
  //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |         NAME_TRN_ID           |0|  0x0  |0|0|1|0|0 0|B|  0x0  |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |          0x0001               |           0x0000              |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |          0x0000               |           0x0000              |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // /                         QUESTION_NAME                         /
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |           NB (0x0020)         |        IN (0x0001)            |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // The whole packet is 50 bytes long
  unsigned char netbios_query[] = {
    0x12, 0x34, // NAME_TRN_ID
    0x01, 0x10, // Flags
    0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x20,                   // Name length is at byte 12
    0x00, 0x00, 0x00, 0x00, // Name starts at byte 13
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 
    0x00, 0x20, // NB
    0x00, 0x01  // IN
  };
  // Encode the name into the packet
  encode_netbios_name(netbios_query+13, argv[1]);

  // Create a UDP  broadcast socket
  SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  int bcast = 1;
  if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*) &bcast, sizeof(bcast)) < 0) {
    printf("Error setting broadcast option: %d\n", WSAGetLastError());
    return 1;
  }

  // 2 second timeout
  int timeout = 2000;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

  // Use the broadcast address and the NBT port 137
  struct sockaddr_in broadcast_addr;
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(137);
  broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST; // 255.255.255.255

  // Send the name query broadcast
  sendto(s, netbios_query, sizeof(netbios_query), 0, (struct sockaddr*) &broadcast_addr, sizeof(broadcast_addr));
  printf("Broadcast sent. Waiting for responses ...\n");

  // We may get more than one response so use a loop
  struct sockaddr_in sender_addr;
  int sender_len = sizeof(sender_addr);
  for (;;) {
    // Listen for a reply (2 second timeout)
    int bytes = recvfrom(s, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*) &sender_addr, &sender_len);
    // If the reply timed out exit the loop
    if (bytes == SOCKET_ERROR)
      break;

#ifdef NBT_DEBUGGING
   printf("Received %d bytes\n", bytes);
    dump_packet(recv_buf, bytes);
#endif

  // A valid reply must have at least 62 bytes
  if (bytes < 62) {
    printf("Received less than the required 62 bytes\n");
    continue;
  }

  // Print the IP address
  struct in_addr a;
  memcpy(&(a.s_addr), recv_buf+58, 4);

  char v4addr[16];
  inet_ntop(AF_INET, &a, v4addr, 16);
  printf("Found address: %s\n", v4addr);
}

  // All finished so tidy up
  closesocket(s);
  WSACleanup();

  // And exit indicating success
  return 0;
}