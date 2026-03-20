//----------------------------------------------------------------------
// mDNSListener.cpp
// ================
// Code to listen for and respond to mDNS name requests
//----------------------------------------------------------------------

// Uncomment this for extra debugging info
// #define MDNS_DEBUGGING

#ifdef MDNS_DEBUGGING
// We need the Arduino code only for Serial.print
#include <Arduino.h>
#endif

#include <stdint.h>
#include <string.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include "mDNSListener.h"

#define MDNS_PORT 5353
#define MDNS_ADDR "224.0.0.251"

//----------------------------------------------------------------------
// DNS structures
//----------------------------------------------------------------------
typedef struct _dns_header {
  uint16_t id;     // Should be 0 for mDNS
  uint16_t flags;    // 0x8400 for standard response
  uint16_t q_count;  // Questions
  uint16_t ans_count;  // Answer RRs
  uint16_t auth_count; // Authority RRs
  uint16_t add_count;  // Additional RRs
} dns_header;
#define LEN_DNS_HEADER 12

typedef struct _dns_query {
  uint16_t qtype;
  uint16_t qclass;
} dns_query;
#define LEN_DNS_QUERY 4

typedef struct _dns_reply {
  uint16_t rrtype;
  uint16_t rrclass;
  uint32_t ttl;
  uint16_t rdlength;
  uint32_t rdata;
} dns_reply;
#define LEN_DNS_REPLY 14

// Header for reply packet
const unsigned char mdns_req_reply_hdr[] = {
  0x00, 0x00, // id
  0x84, 0x00, // flags
  0x00, 0x00, // no. questions
  0x00, 0x01, // no. answers
  0x00, 0x00, // no. authority RRs
  0x00, 0x00  // no. additional RRs
};

// Data for reply packet
const unsigned char mdns_req_reply_data[] = {
  0x00, 0x01,             // rrtype
  0x00, 0x01,             // rrclass, high bit is cache flush
  0x00, 0x00, 0x00, 0x3c, // ttl
  0x00, 0x04              // rdlength
};

//----------------------------------------------------------------------
// struct used to pass the name and IP address to the task function
//----------------------------------------------------------------------
typedef struct _mDNSDetails {
  // This is the hostname
  char name[LEN_HOSTNAME+1];
  // This is the v4 IP address
  char address[LEN_IPADDRESS];
  // Used to return an error code
  uint32_t nbterr;
} mDNSDetails;

//----------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------
static char hostname[LEN_HOSTNAME+1]; // 253 chars + `\0`
static char dnsname[LEN_HOSTNAME+3]; // 255 chars + `\0`
static unsigned char buffer[512]; // More than long enough for a DNS packet

#ifdef MDNS_DEBUGGING
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
// ConvertName
// -----------
// Covert the dns name to the form used by mDNS
//----------------------------------------------------------------------
int ConvertName(char* To, const char* From, int BufLen) {
  // Check the output buffer is long enough
  if (strlen(From) + 2 > BufLen)
    return 0;

  // Do the conversion
  int len = 0;
  int from = 0, to = 1;
  for (;;) {
    // Zero the current length byte
    To[len] = 0;
    // Copy in the name up to the next . or end of string
    while (From[from] != '.' && From[from] != '\0') {
      To[to++] = From[from++];
      To[len] += 1;
    }
    // If we hit the end of the string we are done
    if (From[from] == '\0') break;
    // Else set the next length byte
    len = to++;
    from++;
  }

  // Finally terminate the string
  To[to] = '\0';

  // And return the length of the string
  return to;
}

//----------------------------------------------------------------------
// mDNSListener
// ------------
// This is the task that runs the name resolution server
//----------------------------------------------------------------------
static void mDNSListener(void* Parameter) {
  // The parameter is a pointer to an mDNSDetails struct with the
  // Netbios name and associated IP address as strings.
  mDNSDetails* details = (mDNSDetails*) Parameter;
  details->nbterr = MDNS_ERR_OK;

  // Check the name length
  // Since we check the name length here we can be sure there won't be
  // any buffer overflows so we don't need to check lengths below.
  if (strlen(details->name) > LEN_HOSTNAME) {
    details->nbterr = MDNS_ERR_TOOLONG;
    vTaskDelete(NULL);
  }

  // Copy the name
  strcpy(hostname, details->name);
  int len_dnsname  = ConvertName(dnsname, details->name, LEN_HOSTNAME+3);

  // Convert the IP address
  in_addr_t my_addr;
  if (0 == inet_aton(details->address, &my_addr)) {
    details->nbterr = MDNS_ERR_INVALIDIP;
    vTaskDelete(NULL);
  }

  // Create the socket and enable reuse
  int sockfd = lwip_socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    details->nbterr = MDNS_ERR_CREATE;
    vTaskDelete(NULL);
  }
  // Allow address reuse to prevent "Address already in use" errors
  int reuse = 1;
  lwip_setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuse, sizeof(reuse));

  // Listen on the mDS port
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(MDNS_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (0 != lwip_bind(sockfd, (struct sockaddr*) &addr, sizeof(addr))) {
    lwip_close(sockfd);
    details->nbterr = MDNS_ERR_BIND;
    vTaskDelete(NULL);
  }

  // Join the Multicast Group
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(MDNS_ADDR);
  mreq.imr_interface.s_addr = INADDR_ANY;
  if (0 != lwip_setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &mreq, sizeof(mreq))) {
    lwip_close(sockfd);
    details->nbterr = MDNS_ERR_ADD;
    vTaskDelete(NULL);
  }

  // Now sit in a loop waiting for mDNS queries
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int len = lwip_recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*) &client_addr, &addr_len);
    if (len <= 0) continue;
#ifdef MDNS_DEBUGGING
    Serial.printf("Received %d bytes\n", len);
    dump_packet(buffer, len);
#endif

    // The packet length should be header + name+'\0' + query
    if (len != sizeof(dns_header) + len_dnsname + 1 + sizeof(dns_query)) continue;

    // Check it's a query
    dns_header* qry_hdr = (dns_header*) buffer;
    if (qry_hdr->flags != 0) continue;

    // Check it's a request for an A record
    dns_query* qry = (dns_query*) (buffer + sizeof(dns_header) + len_dnsname + 1);
    if (qry->qtype != ntohs(0x0001)) continue;

    // Check if it's a request for our name
    char* req_name = (char*) (buffer + sizeof(dns_header));
    if (strcasecmp(req_name, dnsname) != 0) continue;

    // We have received a request for our name
#ifdef MDNS_DEBUGGING
    Serial.printf("Received A request for %s\n", hostname);
#endif

    // Construct the reply, copy in the header
    memcpy(buffer, mdns_req_reply_hdr, sizeof(mdns_req_reply_hdr));
    int len_reply = sizeof(mdns_req_reply_hdr);
    // Copy in the name
    memcpy(buffer + len_reply, dnsname, len_dnsname+1); // include the '\0'
    len_reply += len_dnsname + 1;
    // Copy in the data
    memcpy(buffer + len_reply, mdns_req_reply_data, sizeof(mdns_req_reply_data));
    len_reply += sizeof(mdns_req_reply_data);
    // Finally copy in the IP address
    memcpy(buffer + len_reply, &my_addr, 4);
    len_reply += 4;
#ifdef MDNS_DEBUGGING
    Serial.printf("Replying with:\n");
    dump_packet(buffer, len_reply);
#endif

    // Send the reply to the multicast address
    struct sockaddr_in reply_addr = {0};
    reply_addr.sin_family = AF_INET;
    reply_addr.sin_port = htons(MDNS_PORT);
    reply_addr.sin_addr.s_addr = inet_addr(MDNS_ADDR);
    len = lwip_sendto(sockfd, buffer, len_reply, 0, (struct sockaddr*) &reply_addr, sizeof(reply_addr));
#ifdef MDNS_DEBUGGING
    if (len != len_reply)
      Serial.printf("Error sending name request, errno = %d\n", errno);
#endif
  }
}

//----------------------------------------------------------------------
// StartmDNSListener
// -----------------
// Function that starts the name query listener
// This is the only function the ESP32 sketch needs to call
//----------------------------------------------------------------------
int StartmDNSListener(const char* Name, const char* v4IPAddr) {
  // Copy the details into a struct so we can pass them to the new task
  mDNSDetails details;
  strlcpy(details.name, Name, LEN_HOSTNAME);
  strlcpy(details.address, v4IPAddr, LEN_IPADDRESS);

  // Create the task
  TaskHandle_t hmDNSListener;
  xTaskCreate(
    mDNSListener,
    "mDNSListener",
    4000, // from experiment 4k seems enough
    (void*) &details,
    1,
    &hmDNSListener
  );

  // Wait a moment to let the task start
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Return the error code from the task
  return details.nbterr;
}