# PingTest

This is a test app to demonstrate how to send pings from an ESP32.

Three different methods are provided. The aim is that you can choose whatever method suits you and just drop the source files into your sketch.

### ESP-IDF Ping Session

The ESP-IDF provides built in support for ping but it works in a slight eccentric way. The IDF provides functions for managing a [ping session](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/icmp_echo.html). This starts a ping once a second that runs in the background accumulating statistics as it runs. Once the ping session is started it runs independently of your program until you stop it (I'm not sure if it runs as a separate task or as part of your task using interrupts).

I have wrapped the ping session code in a class to simplify it. You use it like this:

`PingSession p;`  
to create the object (the constructor has no arguments) then:

`p.start("www.espressif.com", n);`  
to start the pings and wait for `n` replies (or timeouts). If `n` is zero the function returns immediately leaving the ping session running in the background. The ping session runs until you call `p.stop()`.  
`p.start()` returns `true` if it succeeds or `false` if there is an error. In case of an error you can call `p.last_error_meg()` to return a pointer to the error message.

At any point after starting the ping session you can call one of the following to return information about the pings:

```
p.sent()            - number of pings sent
p.replies()         - number of replies
p.timeouts()        - number of timeouts
p.last_reply_time() - the reply time of the last ping sent
p.min_reply_time()  - the minimum reply time
p.max_reply_time()  - the maximum reply time
p.av_reply_time()   - the average reply time
```

If you need it `p.ip_address()` returns the IP address that the host name resolved to as a string.

To use this just drop ``PingSession.cpp` and ``PingSession.h` into your project and #include `PingSession.h`.

### LWIPPing class

This is a far simpler implementation than the ping session. It uses ping code that I had from my BSD days in the 1980s rewritten to use the ESP32's LWIP API. It only has the two methods:

```
LWIPPing p;
reply_time = p.ping_host("www.espressif.com", 1000);
reply_time = p.ping_ip("8.8.8.8", 1000);
```

to ping either a host name or an IP address passed as strings, with a timeout of (in this case) 1000 milliseconds.

The `ping` functions return a reply time in milliseconds if the ping succeeds, or a negative error code if the ping fails. You can get the related error message using `p.last_error_msg()`, or the error is one of:

```
#define LWIPPING_ERR_TIMEOUT  -1 // ping timed out
#define LWIPPING_ERR_HOSTNAME -2 // host name not found
#define LWIPPING_ERR_IPADDR   -3 // invalid IP address
#define LWIPPING_ERR_OTHER    -4 // all other errors
```

To use this just drop `LWIPPing.cpp` and `LWIPPing.h` into your project and #include `LWIPPing.h`.

### LWIP C code

This is the same code as for the `LWIPPing` class but written as C functions. There are only two functions:

```// Ping a host name (requires DNS)
int ping_host(const char* hostname, int timeout);
// Ping an IP address passed as a string (does not require DNS)
int ping_ip(const char *ip_addr, int timeout);
```

plus a `ping_last_error_msg()` function to return the last error message. As above, the pings return the reply time or one of the following error numbers:

```
#define PING_ERR_TIMEOUT  -1 // ping timed out
#define PING_ERR_HOSTNAME -2 // host name not found
#define PING_ERR_IPADDR   -3 // invalid IP address
#define PING_ERR_OTHER    -4 // all other errors
```

To use this just drop `ping.c` and `ping.h` into your project and #include `ping.h`.
