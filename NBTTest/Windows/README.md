# nbtlookup

This is a Windows command line app to send Netbios name query requests. The syntax is:

```
nbtlookup <target_name>
```

If the host responds to the request its IP address will be printed. If you don't get a reply then try again. Responses to broadcasts can be a bit erratic.

The app does the same as the built in Windows command:

```
nbtstat -a <target_name>
```

# Building the app

This app is written for the Microsoft Visual Studio development environment. You can use either the full Visual Studio (the free Community Edition works fine) or the command line only Build Tools for Visual Studio (which is what I used). To build the app use:

```
cl nbtlookup.c
```

The version included here is the 64 bit build.

# How it works

RFC1002 describes the Netbios name query mechanism. The app sends a _Name Query Request_ packet to the network broadcast address containing the requested name. The host that has that name should respond by sending back a _Positive Name Query Response_ packet containing its IP address. The app receives this packet and extracts and print the IP address.