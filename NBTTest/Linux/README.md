# nbtlookup

_Note that the Linux app is currently untested. All volunteers to test it are welcome._

This is a Linux app to send Netbios name query requests. The syntax is:

```
nbtlookup <target_name>
```

If the host responds to the request its IP address will be printed. If you don't get a reply then try again. Responses to broadcasts can be a bit erratic.

### Building the app

To build the app use:

```
gcc -o nbtlookup nbtlookup.c
```

### How it works

RFC1002 describes the Netbios name query mechanism. The app sends a _Name Query Request_ packet to the network broadcast address containing the requested name. The host that has that name should respond by sending back a _Positive Name Query Response_ packet containing its IP address. The app receives this packet and extracts and print the IP address.