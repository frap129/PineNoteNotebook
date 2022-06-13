The Pen provides x, y, and pressure.

The top of the device is where the pen magnetically attaches to the device.

The bottom left corner of the display is (0, 0).

The y coordinate increases as you move right.

The x coordinate increases as you move up.

The top right corner of the display is (1872, 1404).

This means that the width of the display is 1404 pixels and the height is 1872 pixels.

The display driver considers the width to be 1872 pixels and the height to be 1404 pixels.

So this means that we need to rotate the pen event coordinates by 90 degrees to get the correct coordinates.