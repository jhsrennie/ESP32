# NBTListener

The code provides a way for you to find your ESP32 on a network.

The wireless network you are using may not support dynamic DNS and you may not have access to the DHCP server. In that case it can be hard to find out what IP address your ESP32 has been allocated. This code provides a simple and low overhead way for you to find your ESP32.

This problem is as old as networks, and back in the 1980s a solution was provided by the Netbios, or more specifically Netbios over TCP/IP (NBT), protocol described in RFC1001 and RC1002. NBT allows you to send a _Name Query Request_ broadcast packet containing the name of the host, and that host should reply with a _Name Query Response_ packet containing its IP address (or addresses if it has more than one). This is little used these days as on large networks the broadcast traffic involved can be a significant load on the network, however it does provide a simple way to perform a name lookup without a DNS server.

The code here implements a name query listener that responds to NBT name queries and allows you to find your ESP32. It is designed to be simple an lightweight. Just add the files `NBTListener.cpp` and `NBTListener.h` to your project and call:

```
StartNBTListener(name, ip_address);
```

to start the listener. You can make up any name you want and you need the IP address from your Wi-Fi adaptor. For example if you are using the Arduino IDE, as the example sketch here does, you can call:

```
int e = StartNBTListener("foobar", WiFi.localIP().toString().c_str());
```

then you can find your ESP32 by searching for the name _foobar_.

If you are using Windows then you can use the built in `nbtstat` command to perform the search. For example:

```
nbtstat -a foobar
```

will send an NBT name query for the name _foobar_ and list the ip addresses found. Alternatively I have written a command line app called `nbtlookup` that does a similar search. There is a version for Windows in the `./Windows` directory and a version for Linux in the `./Linux` directory.

The listener code is pretty lightweight. It adds about 2.5KB to your binary and uses negligible CPU. The listener runs as a separate task so once started it will sit in the background allowing you to get on with the important stuff.

NBT name queries support both IPv4 and v6. The code here implements only the v4 queries to keep the code lightweight. Modification to support v6 would be straightforward if you needed it for your application.