# TronPOV

This is the Arduino sketch for a handheld Persistence of Vision (POV) display I built to display the logo from the 2010 movie [TRON: Legacy](http://www.imdb.com/title/tt1104001/). 

![TRON POV](http://www.tomtilley.net/projects/tron-pov/images/box_t.jpg)

The display uses 10 blue LEDs and is powered by a 9v battery.  Inside the box is an Arduino Duemilanove with an [ADXL345 three axis accelerometer](https://www.sparkfun.com/products/9836).  When you wave the box from side to side through the air it displays the movie logo and a switch on the front can be used to change patterns.  You can find more details about building the display including a circuit diagram on [my website](http://www.tomtilley.net/projects/tron-pov/).

![TRON Logo](http://www.tomtilley.net/projects/tron-pov/images/tron-pov_t.jpg)

Each pattern is stored in a separate header file that contains an array of words which are each composed of two bytes representing the pixels to be displayed.  For example, we could represent a 9 pixel high letter 'I' like this where a '`1`' represents an LED that is turned on and a '`0`' is off:

```
***          111
 *     |\    010
 *   +-+ \   010
 *   |    >  010
 *   +-+ /   010
 *     |/    010
***          111
             000
```

Each of these columns would be displayed in turn on the 10 LEDs as the display moves through the air.  However, each column of pixels is stored as a row in the pattern array. Rotating the letter 90 degrees clockwise it looks like this:

```
0100000001
0111111111
0100000001
```
Each row is then broken into two groups of five bits and each group is represented by a single byte.  For example, inside the header file '[i_heart_u.h](https://github.com/Tominator2/TronPOV/blob/master/tron_pov/i_heart_u.h)' the letter 'I'  is represented by the following rows:
```
 ...
 word(B01000,B00001), // I
 word(B01111,B11111),
 word(B01000,B00001),
 ...
```

The reason the 'I' is only 9 pixels high (instead of using all 10) is because the bottom of the heart next to it looked better if it ended at a sharp single pixel point.

The sketch uses the Arduino [Wire library](http://www.arduino.cc/en/Reference/Wire) to communicate with the accelerometer using I<sup>2</sup>C and that part of the code was based on [this blog post](http://codeyoung.blogspot.com/2009/11/adxl345-accelerometer-breakout-board.html) from [Live Fast - Code Young](http://codeyoung.blogspot.com/). 
