# ColorClock
Colorful DIY clock with 60 Neo-LEDs.
This is a project for the VHS Esslingen - Technikschule.
In the project we are building up a real clock with one NEO RGB LED for each Minute or Second.

Here a picture of a early version
![Early Version](https://github.com/maBurger/ColorClock/blob/master/media/ColorClockV0.2.jpg)


## Hardware
1. Standard arduino
2. Neo RGB LED strip with 60 LED per Meter
3. 40cm x 40cm MDF boards

## Software
Following requirements are needed

***Libraries***
* [TimerOne][TimerOne] for getting Timer Interupts for the seconds
* [Adafruit Neo Pixel][AdaNeoPix] for driving the LED strip we have in use.

[AdaNeoPix]: https://github.com/adafruit/Adafruit_NeoPixel
[TimerOne]: https://www.pjrc.com/teensy/td_libs_TimerOne.html