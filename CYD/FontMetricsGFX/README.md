FontMetricsGFX
==============
This is a sketch to measure the sizes in pixels of the Arduino GFX fonts when rendered on a CYD. The free fonts used by TFT_eSPI appear to be the same sizes so this applies to both libraries.

When you call the GFX printf() method the line depth is the spacing between lines to make the text look good. That is, if you print a new line (\n) the depth is the number of pixels the cursor moves down.

The char depth is the minimum vertical spacing required between lines to ensure no overlap. This is less than the line depth because text rendered with this spacing looks cramped and the library adds extra space to the line depth to make the text look better.

When you call printf() the text is drawn so the base of letters like "A" that have no descenders aligns with the cursor. Characters with descenders like "y" will be drawn below the cursor. The descender is the number of pixels below the base that you have to avoid clipping the descenders. That is, to clear the line the text is on use:

fillRect(0, getCursorY() - Depth + Descender, SCREEN_WIDTH, Depth, BACKGROUND_COLOUR);

The depths and offsets are defined in FontMetricsGFX.h so this can be included in your program.

                 Depth   Descender
FreeSans9pt7b      22        5
FreeSans12pt7b     29        6
FreeSans18pt7b     42        9
FreeSans24pt7b     56       11
