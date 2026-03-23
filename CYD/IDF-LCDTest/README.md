# IDF-LCDTest

Connecting displays to an ESP32 can be a messy business as there are many different types of display and they all work in different ways. In an operating system like Windows the OS abstracts away all the messy details so for example you can use the WIN32 function `LineTo()` to draw a line without worrying what graphics card the PC has.

However the OS on the ESP32 is so cut down that with one exception no OS graphics functions exist. That exception is the `esp_lcd_panel_draw_bitmap()` function described in the [Espressif LCD article](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html), and all that function can do is blit a bitmap to the screen, so it isn't very useful.

But if you're curious this sketch shows how to set up and use this single graphics function on the CYD. But I must emphasise this is for curiousity only as this really isn't very useful as it is so limited. Most of us would just use a graphics library like TFT_eSPI.

Some comments:

The `esp_lcd_panel_draw_bitmap()` function is described as:

```
esp_lcd_panel_draw_bitmap(
  esp_lcd_panel_handle_t panel,
  int x_start,
  int y_start,
  int x_end,
  int y_end,
  const void * color_data);
```
The origin is at `(0,0)` as you'd expect but note that the `end` coordinates are one greater than you think. Suppose you have a 10x10 bitmap that you want to draw you'd call:

```
esp_lcd_panel_draw_bitmap(panel, x, y, x+10, y+10, data);
```
i.e. the `end` coordinates are the pixel just past the edges of the bitmap.