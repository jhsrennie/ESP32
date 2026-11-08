# ILI9341

This is an attempt to use the built in ILI9341 driver provided by LVGL.

This is almost working. It draws part of the display but then freezes. I have added code to test the display is working by writing colours to it using direct SPI commands so I know the display is correctly connected and the SPI bus is correctly configured. But when LVGL starts writing to the screen it crashes.

[The Espressif documentation for this is here](https://lvgl.io/docs/open/integration/external_display_controllers/ili9341).