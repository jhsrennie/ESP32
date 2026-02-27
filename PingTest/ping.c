//----------------------------------------------------------------------
// ping.c
// ======
// Ping implementation for the ESP32 in C
//----------------------------------------------------------------------
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <lwip/icmp.h>
#include <lwip/ip.h>
#include <lwip/netdb.h>
#include <lwip/sys.h>
#include "ping.h"

//----------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------

#define ICMP_ECHO 8

#define STR_ERR_TIMEOUT  "Request timed out"
#define STR_ERR_HOSTNAME "Host name not found"
#define STR_ERR_IPADDR   "Invalid IP address"
#define STR_ERR_OTHER    "Unknown error"

static int last_error;
int ping_last_error() { return last_error; }

#define MAX_ERRMSG 256
char last_error_msg[MAX_ERRMSG];
const char* ping_last_error_msg() { return last_error_msg; }

//----------------------------------------------------------------------
// calculate_checksum
// ------------------
// Calculate the checksum used in the ICMP header
//----------------------------------------------------------------------
static uint16_t calculate_checksum(void *dataptr, uint16_t len) {
  uint32_t acc = 0;
  uint16_t *data = (uint16_t *)dataptr;

  while (len > 1) {
    acc += *data++;
    len -= 2;
  }
  if (len > 0) {
    acc += *(uint8_t *)data;
  }
  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  acc += (acc >> 16);
  return (uint16_t)~acc;
}

//----------------------------------------------------------------------
// check_reply
// -----------
// Non-class function to check that the buffer holds an ICMP echo reply
// matching the id.
//----------------------------------------------------------------------
static bool check_reply(uint16_t id, char* buffer, int buflen) {
  // The first part of the buffer is the IP header
  if (buflen < sizeof(struct ip_hdr))
    return false;

  struct ip_hdr *iphdr = (struct ip_hdr*) buffer;
  int ip_hdr_len = IPH_HL(iphdr)*4;

  // The next part is the echo header
  if (buflen < ip_hdr_len + sizeof(struct icmp_echo_hdr))
    return false;

  struct icmp_echo_hdr *reply_hdr = (struct icmp_echo_hdr*) (buffer + ip_hdr_len);

  // We have the header so check it's a reply with our id
  if (reply_hdr->id != id || reply_hdr->type != 0) // Type 0 = Echo Reply
    return false;

  // Return indicating it's a valid response
  return true;
}

//----------------------------------------------------------------------
// lwip_ping_sub
// -------------
// Non-class function to ping an Internet network address
//----------------------------------------------------------------------
static int lwip_ping_sub(in_addr_t s_addr, int timeout) {
  // The packet length we use is header + 32 bytes of data
  int packet_len = sizeof(struct icmp_echo_hdr) + 32;

  // Create the socket - on failure set_errno(ENOBUFS)
  int s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
  if (s < 0)
    return PING_ERR_OTHER;

  // Set the destination address
  struct sockaddr_in dest_addr = {0};
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = s_addr;

  // Generate a packet signature from the current time
  uint16_t packet_sig = (uint16_t) (sys_now() & 0xFFFF);
  // This holds the ICMP packet i.e. header+data
  char buffer[64];
  memset(buffer, 0, sizeof(buffer));
  // The header is at the start of the packet
  struct icmp_echo_hdr* echo_hdr = (struct icmp_echo_hdr*) buffer;
  echo_hdr->type  = ICMP_ECHO;
  echo_hdr->code  = 0;
  echo_hdr->id    = packet_sig;
  echo_hdr->seqno = 1;

  // Fill payload with dummy data
  for(int i = sizeof(struct icmp_echo_hdr); i < packet_len; i++)
    buffer[i] = (char) i;
  echo_hdr->chksum = calculate_checksum(buffer, packet_len);

  // Send Packet
  uint32_t start_time = sys_now();
  if (lwip_sendto(s, buffer, packet_len, 0, (struct sockaddr*) &dest_addr, sizeof(dest_addr)) <= 0) {
    lwip_close(s);
    return PING_ERR_OTHER;
  }

  // Wait for the reply
  bool got_reply = false;
  uint32_t end_time = sys_now() + timeout;
  while (sys_now() < end_time) {
    // Set the timeout to the time remaining
    uint32_t remaining = end_time - sys_now();
    struct timeval tv = {
      .tv_sec = remaining/1000,
      .tv_usec = (remaining % 1000)*1000
    };
    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Read the reply
    struct sockaddr_in from_addr = {0};
    socklen_t from_len = sizeof(from_addr);
    int n = lwip_recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr*) &from_addr, &from_len);

    // Check the signature to make sure it's a reply to our ping
    if (check_reply(packet_sig, buffer, n)) {
      got_reply = true;
      break;
    }

    // Otherwise we loop back to try again
  }

  // All done so close the socket
  lwip_close(s);

  // The function returns the reply time in ms or timeout if there was no reply
  return got_reply ? sys_now() - start_time : PING_ERR_TIMEOUT;
}

//----------------------------------------------------------------------
// ping_host
// ---------
// Ping a host name passed as a string
// The timeout is in milliseconds
//----------------------------------------------------------------------
int ping_host(const char* hostname, int timeout) {
  last_error = 0;
  last_error_msg[0] = '\0';

  // Convert host name to IP address
  struct addrinfo hint = {0};
  struct addrinfo *res = NULL;
  int e = getaddrinfo(hostname, NULL, &hint, &res);
  if (e != 0) {
    last_error = PING_ERR_HOSTNAME;
    strlcpy(last_error_msg, STR_ERR_HOSTNAME, MAX_ERRMSG);
    freeaddrinfo(res);
    return last_error;
  }

  // Extract the IP address
  struct sockaddr_in* dest_addr = (struct sockaddr_in*) res->ai_addr;
  in_addr_t in_addr = dest_addr->sin_addr.s_addr;
  freeaddrinfo(res);

  // And ping the address
  last_error = lwip_ping_sub(in_addr, timeout);
  if (last_error == PING_ERR_TIMEOUT)
    strlcpy(last_error_msg, STR_ERR_TIMEOUT, MAX_ERRMSG);
  else if (last_error == PING_ERR_OTHER)
    strlcpy(last_error_msg, STR_ERR_OTHER, MAX_ERRMSG);
  return last_error;
}

//----------------------------------------------------------------------
// ping_ip
// -------
// Ping an IP address passed as a string
// The timeout is in millseconds
//----------------------------------------------------------------------
int ping_ip(const char *ip_addr, int timeout) {
  last_error = 0;
  last_error_msg[0] = '\0';

  // Convert the ip address string to a network address
  struct sockaddr_in dest_addr = {0};
  dest_addr.sin_family = AF_INET;
  int e = inet_pton(AF_INET, ip_addr, &dest_addr.sin_addr);
  if (e != 1) {
    last_error = PING_ERR_IPADDR;
    strlcpy(last_error_msg, STR_ERR_IPADDR, MAX_ERRMSG);
    return last_error;
  }

  // And ping the address
  last_error = lwip_ping_sub(dest_addr.sin_addr.s_addr, timeout);
  if (last_error == PING_ERR_TIMEOUT)
    strlcpy(last_error_msg, STR_ERR_TIMEOUT, MAX_ERRMSG);
  else if (last_error == PING_ERR_OTHER)
    strlcpy(last_error_msg, STR_ERR_OTHER, MAX_ERRMSG);
  return last_error;
}