# mDNSListener

The code provides a way for you to find your ESP32 on a network using the mDNS protocol.

The wireless network you are using may not support dynamic DNS and you may not have access to the DHCP server. In that case it can be hard to find out what IP address your ESP32 has been allocated. To fix problems like this the [Multicast DNS](https://en.wikipedia.org/wiki/Multicast_DNS) (mDNS) protocol was created.

mDNS is quite a complicated protocol, and [Espressif provide a component](https://github.com/espressif/esp-protocols/tree/master/components/mdns) that implements it in full, however most of the time we just need a quite and easy way to find out device and that's what this sketch demonstrates.

The code here implements an mDNS listener that responds to mDNS name (A record) queries and allows you to find your ESP32. It is designed to be simple and lightweight. Just add the files `mDNSListener.cpp` and `mDNSListener.h` to your project and call:

```
StartmDNSListener(name, ip_address);
```

to start the listener. You can make up any name you want and you need the IP address from your Wi-Fi adaptor. For example if you are using the Arduino IDE, as the example sketch here does, you can call:

```
int e = StartmDNSListener("foobar.local", WiFi.localIP().toString().c_str());
```

then you can find your ESP32 by searching for the name _foobar.local_. If you are using Windows this is straightforward because Windows falls back to a mDNS query if DNS fails. You can just type:

```
ping foobar.local
```

and it will work. Note however that this won't work on Linux and OSX, though you can configure Linus to use mDNS.

For completeness I have written a command line program, `mdnslookup`, for Windows to send mDNS A requests and you'll find this in the Windows subdirectory. There is a Linux version in the Linux subdirectory but note this is untested.

The listener code is pretty lightweight. It adds about 3.5KB to your binary and uses negligible CPU. The listener runs as a separate task so once started it will sit in the background allowing you to get on with the important stuff.

mDNS name queries support both IPv4 and v6, however the code here implements only the v4 queries to keep the code as lightweight as possible.