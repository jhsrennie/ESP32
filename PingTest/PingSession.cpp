//----------------------------------------------------------------------
// PingSession.cpp
// ===============
// Class to ping a host using the ESP-IDF ping session fuctions
// For more see:
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/icmp_echo.html
//----------------------------------------------------------------------
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <ping/ping_sock.h>
#include "PingSession.h"

//----------------------------------------------------------------------
// This is a group of functions used as callbacks by the ping session
//----------------------------------------------------------------------
static void on_ping_success(esp_ping_handle_t hdl, void *args)
{
  // args is a pointer to the Ping object
  PingSession* ping = (PingSession*) args;

  // Get the reply time
  int reply_time;
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &reply_time, sizeof(reply_time));

  // And save the time of the last reply
  ping->inc_replies(reply_time);
}

static void on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
  // args is a pointer to the Ping object
  PingSession* ping = (PingSession*) args;

  // Increment the number of timeouts
  ping->inc_timeouts();
}

static void on_ping_end(esp_ping_handle_t hdl, void *args)
{
  // Unused at the moment
}

//----------------------------------------------------------------------
// Constructor
// -----------
//----------------------------------------------------------------------
PingSession::PingSession() {
  // Reset the handle
  ping_handle = 0;
  // Reset the target address
  target_addr = {0};
  // Reset the sesson statistics
  reset();
}

PingSession::~PingSession() {
  // If a ping session is active stop and delete that session
  if (ping_handle) {
    esp_ping_stop(ping_handle);
    esp_ping_delete_session(ping_handle);
  }
}

//----------------------------------------------------------------------
// start
// -----
// This starts pinging the host. The pings will continue indefinitely
// until stop() is called.
// If the wait argument is non-zero the function will wait for that
// number of replies or timeouts before returning.
//----------------------------------------------------------------------
bool PingSession::start(const char* host, int wait) {
  // If a ping session is active stop and delete that session
  stop();

  // Clear the last error message
  errmsg[0] = 0;

  // Convert host name to IP address
  struct addrinfo hint = {0};
  struct addrinfo *res = NULL;
  last_error = getaddrinfo(host, NULL, &hint, &res);

  // If the name resolution failed save the error message the return false
  if (last_error != 0) {
    strlcpy(errmsg, "Host name not found", MAX_ERRMSG);
    freeaddrinfo(res);
    return false;
  }

  // Extract the IP address
  struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
  inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
  freeaddrinfo(res);

  // Set up the ping sesssion config
  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  ping_config.target_addr = target_addr;          // target IP address
  ping_config.count = ESP_PING_COUNT_INFINITE;    // ping in infinite mode, esp_ping_stop can stop it

  // Set up the ping sesssion callbacks
  esp_ping_callbacks_t cbs;
  cbs.on_ping_success = on_ping_success;
  cbs.on_ping_timeout = on_ping_timeout;
  cbs.on_ping_end     = on_ping_end;
  cbs.cb_args         = (void*) this;

  // Create the ping session
  ping_handle = 0;
  esp_err_t e = esp_ping_new_session(&ping_config, &cbs, &ping_handle);
  if (e != ESP_OK) {
    strlcpy(errmsg, "Failed to create ping session", MAX_ERRMSG);
    return false;
  }

  // And start the session
  e = esp_ping_start(ping_handle);
  if (e != ESP_OK) {
    strlcpy(errmsg, "Failed to start ping session", MAX_ERRMSG);
    return false;
  }

  // If we have been asked to wait for replies do that now
  if (wait > 0)
    while (wait > num_replies + num_timeouts)
      vTaskDelay(pdMS_TO_TICKS(100));

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// stop
// ----
// This stops ping the host
//----------------------------------------------------------------------
void PingSession::stop() {
  // If a ping session is active stop and delete that session
  if (ping_handle) {
    esp_ping_stop(ping_handle);
    esp_ping_delete_session(ping_handle);
    ping_handle = NULL;
  }
}

//----------------------------------------------------------------------
// reset
// -----
// This resets the statistics
//----------------------------------------------------------------------
void PingSession::reset() {
  num_replies = num_timeouts = 0;
  ping_last_reply_time  = -1;
  ping_min_reply_time   = 666666; // Arbitrary large number
  ping_max_reply_time   = -1;
  total_reply_time = 0;
}

//----------------------------------------------------------------------
// ip_address
// ----------
// Return the target IP address as a string
//----------------------------------------------------------------------
const char* PingSession::ip_address() {
  static char addr[16];
  uint8_t* arr = (uint8_t*) &target_addr;
  snprintf(addr, 16, "%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);
  return addr;
}

//----------------------------------------------------------------------
// av_reply_time
// -------------
// Return the average reply time
//----------------------------------------------------------------------
int PingSession::av_reply_time() {
  return num_replies > 0 ? total_reply_time/num_replies : -1;
}

//----------------------------------------------------------------------
// These functions are meant to be called by the callbacks
//----------------------------------------------------------------------
void PingSession::inc_replies(int reply_time) {
  total_reply_time += reply_time;
  num_replies++;

  ping_last_reply_time = reply_time;
  if (ping_min_reply_time > reply_time)
    ping_min_reply_time = reply_time;
  if (ping_max_reply_time < reply_time)
    ping_max_reply_time = reply_time;
}

void PingSession::inc_timeouts() {
  num_timeouts++;
}