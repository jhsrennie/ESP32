### Touch

This shows how to use the touchscreen on the CYD. To compile this you need to install the XPT2046_Touchscreen by Paul Stoffregen library.

The sketch includes code to do interrupt handling but it is commented out as I found it wasn't very useful. You cannot use any library function in the interrupt handler or it crashes in unpredictable ways. I found it better to just poll the touchscreen.