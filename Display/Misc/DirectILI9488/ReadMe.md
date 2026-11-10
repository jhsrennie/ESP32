# DirectILI9488

This shows how an ILI9488 display can be controlled by directly writing commands to the device i.e. without using any graphics library.

This is more for interest than for any useful application since doing anything useful this way would be a great deal of work. Still, it's interesting to see how it all works at a low level.

Unlike the smaller displays the ILI9488 does not support 16 bit colour over SPI. It only supports 18 or 24 bit colour. For convenience this example uses 24 bit colour, but note that this will be slow due to having to transfer an extra byte for each pixel.
