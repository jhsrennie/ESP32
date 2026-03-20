//----------------------------------------------------------------------
// mdnslookup
// ---------
// App to perform mDNS A record lookups.
//----------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

// Uncomment this for extra debugging info
// #define NBT_DEBUGGING

// Syntax
#define SYNTAX "Syntax: mdnslookup <name>\n"

// mDNS constants
#define MDNS_PORT 5353
#define MDNS_ADDR "224.0.0.251"
#define MAX_FQDN 253 // Max length of a fully qualified domain name

// This is the mDNS header for a query
const char mdns_req_hdr[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

// This is the mDNS query block for an A record
const char mdns_req_qry[] = {
    0x00, 0x01, 0x00, 0x01
};

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
// main
// ----
//----------------------------------------------------------------------
#define LEN_MDNSBUF 512 // Longer than the max possible packet
int main(int argc, char* argv[]) {
  unsigned char mdns_buf[LEN_MDNSBUF];

  // Check the arguments
  if (argc != 2) {
    printf(SYNTAX);
    return 1;
  }

  if (strcmp(argv[1], "-?") == 0) {
    printf(SYNTAX);
    return 0;
  }

  // Check the name length
  if (strlen(argv[1]) > MAX_FQDN) {
    printf("The name \"%s\" is too long\n", argv[1]);
    return 1;
  }

  // Convert the name to DNS request format
  char dns_name[MAX_FQDN+3];
  int len_name = ConvertName(dns_name, argv[1], MAX_FQDN+3);
  printf("Sending mDNS name request for %s\n", argv[1]);

  // Create the socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    printf("Cannot create socket, errno = %d\n", errno);
    return 1;
  }
  // Allow reuse
  int reuse = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuse, sizeof(reuse));
  // 2 second timeout
  int timeout = 2000;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));

  // Listen on the mDNS port
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(MDNS_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  int i = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
  if (i != 0) {
    printf("Error calling bind, errno = %d\n", errno);
    return 1;
  }

  // Join the multicast Group
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(MDNS_ADDR);
  mreq.imr_interface.s_addr = INADDR_ANY;
  i = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &mreq, sizeof(mreq));
  if (i != 0) {
    printf("Error joining multicast group, errno = %d\n", errno);
    return 1;
  }

  // Now we need to construct the request. Start with the header.
  int len_req = 0;
  memcpy(mdns_buf, mdns_req_hdr, sizeof(mdns_req_hdr));
  len_req += sizeof(mdns_req_hdr);

  // Append the name
  memcpy(mdns_buf + len_req, dns_name, len_name + 1);
  len_req += len_name + 1; // len_name does not include the terminating '\0'

  // Finally append the query block
  memcpy(mdns_buf + len_req, mdns_req_qry, sizeof(mdns_req_qry));
  len_req += sizeof(mdns_req_qry);

  // Send the request to the multicast address
  struct sockaddr_in req_addr = {0};
  req_addr.sin_family = AF_INET;
  req_addr.sin_port = htons(MDNS_PORT);
  req_addr.sin_addr.s_addr = inet_addr(MDNS_ADDR);
  int len = sendto(sockfd, mdns_buf, len_req, 0, (struct sockaddr*) &req_addr, sizeof(req_addr));
  if (len != len_req) {
    printf("Error sending name request, errno = %d\n", errno);
    return 1;
  }

  // Now listen for replies
  for (;;) {
    struct sockaddr_in reply_addr;
    socklen_t reply_addr_len = sizeof(reply_addr);
    int len = recvfrom(sockfd, mdns_buf, LEN_MDNSBUF, 0, (struct sockaddr*) &reply_addr, &reply_addr_len);
    if (len <= 0) break;
#ifdef NBT_DEBUGGING
    dump_packet(mdns_buf, len);
#endif

    // The length of the data received must be at least the header plus
    // the length of the name
    if (len < sizeof(mdns_req_hdr) + len_name+1) continue;

    // In a valid reply offset 2 is 0x84
    if (mdns_buf[2] != 0x84) continue;

    // Check the name in the reply matches
    if (strcmp(dns_name, mdns_buf+sizeof(mdns_req_hdr)) != 0) continue;

    // Print the IP address
    int off_addr = sizeof(mdns_req_hdr) + len_name+1 + sizeof(mdns_req_qry) + 6;
    struct in_addr a;
    memcpy(&(a.s_addr), mdns_buf + off_addr, 4);

    char v4addr[16];
    inet_ntop(AF_INET, &a, v4addr, 16);
    printf("Found address: %s\n", v4addr);
  }

  // All finished so tidy up
  setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char*) &mreq, sizeof(mreq));
  close(sockfd);

  // And exit indicating success
  return 0;
}