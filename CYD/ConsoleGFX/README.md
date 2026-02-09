ConsoleGFX
==========
This is a class to implement a simple console on a CYD. The class has no cursor position functions but it is an easy way to print output on the screen e.g. for debugging.

You simply add CYDConsole.cpp and .h to your sketch then all you need to write text to the console is:

CYDConsole con;
con.init();
con.printf("foo");
