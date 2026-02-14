ConsoleGFX
==========
This is a class to implement a simple console on a CYD. The class has no cursor position functions but it is an easy way to print output on the screen e.g. for debugging.

You simply add CYDConsole.cpp and .h to your sketch then all you need to write text to the console is:

CYDConsole con;
con.init();
con.printf("foo");

This uses the Adafruit ILI9341 library. See the note on the screen issue with the CYD in the CYD/README.md directory.