# mdnslookup

This is a Windows command line app to send mDNS A record requests. The syntax is:

```
mdnslookup <target_name>
```

In Windows you can also do the lookup using:

```
ping <target_name>
```

as Windows falls back to mDNS name queries if DNS fails.

# Building the app

This app is written for the Microsoft Visual Studio development environment. You can use either the full Visual Studio (the free Community Edition works fine) or the command line only Build Tools for Visual Studio (which is what I used). To build the app use:

```
cl mdnslookup.c
```

The version included here is the 64 bit build.