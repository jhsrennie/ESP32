FontMetricsGFX
==============
This is a sketch to measure the sizes in pixels of the Arduino GFX fonts when rendered on a CYD.

When you call the GFX printf() method the depth is the spacing needed between lines to ensure no overlap. That is, if you print a new line (\n) the depth is the number of pixels the cursor moves down.

When you call printf() the text is drawn so the base of letters like "A" that have no descenders aligns with the cursor. Characters with descenders like "y" will be drawn below the cursor. The vOffset is the number of pixels below the base that you have to add so that there is exactly 1 pixel below the lowest descender. That is, to clear the line the text is on use:

fillRect(0, getCursorY() - Depth + vOffset, SCREEN_WIDTH, Depth, BACKGROUND_COLOUR);

The depths and offsets are defined in FontMetricsGFX.h so this can be included in your program.

                 Depth   vOffset
FreeSans9pt7b      22        6
FreeSans12pt7b     29        7
FreeSans18pt7b     42       10
FreeSans24pt7b     56       12
