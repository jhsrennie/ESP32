//----------------------------------------------------------------------
// PingSession.h
// =============
// Class to ping a host using the ESP-IDF ping session fuctions
// For more see:
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/icmp_echo.html
//----------------------------------------------------------------------
#ifndef _INC_PINGSESSION
#define _INC_PINGSESSION

class PingSession {
  public:
    PingSession();
    ~PingSession();

    bool start(const char* host, int wait = 0);
    void stop();
    void reset();

    void inc_replies(int reply_time);
    void inc_timeouts();

    int sent() const { return num_replies + num_timeouts; };
    int replies() const { return num_replies; };
    int timeouts() const { return num_timeouts; };
    int last_reply_time() const { return ping_last_reply_time; };
    int min_reply_time() const { return ping_min_reply_time; };
    int max_reply_time() const { return ping_max_reply_time; };
    int av_reply_time();

    const char* ip_address();

    const char* last_error_msg() const { return errmsg; };

  private:
    // This is the ping session handle. We declare it as void* so any
    // code using this class doesn't have to include the IDF headers.
    void* ping_handle;

    // The target IPv4 address
    ip_addr_t target_addr;

    // Ping statistics
    int num_replies, num_timeouts;
    // Reply time statistics
    int ping_last_reply_time, ping_min_reply_time, ping_max_reply_time;
    int total_reply_time;

    // Last error message
    #define MAX_ERRMSG 256
    int last_error;
    char errmsg[MAX_ERRMSG];
};

#endif // _INC_PINGSESSION
