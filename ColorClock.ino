#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

uint8_t Sekunden = 10, Minuten = 2, Stunden = 1;
uint8_t striche[12] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

uint32_t Sek_Color, Min_Color, Std_Color, stricheColor;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  strip.setBrightness(25);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Sek_Color = strip.Color(0, 255, 0); // Green
  Min_Color = strip.Color(0, 0, 255); // Blue
  Std_Color = strip.Color(255, 0, 0); // Red
  stricheColor = strip.Color(20, 20, 20);
  
  updateTime();
  
  Timer1.initialize(1000000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
}

void loop(){
}

void updateTime() {
  uint32_t help_Color;
  
  clearStrip();
  setStriche();
  
  strip.setPixelColor(Sekunden, Sek_Color);
  
  // Farbe für Minuten erzeugen
  help_Color = strip.getPixelColor(Minuten);
  help_Color |= Min_Color;
  strip.setPixelColor(Minuten, help_Color);

  // Farbe fuer Stunden erzeugen
  help_Color = strip.getPixelColor(map(Stunden % 12, 0, 12, 0,60));
  help_Color |= Std_Color;
  strip.setPixelColor(map(Stunden % 12, 0, 12, 0,60), help_Color);
  strip.show();
}

void setStriche(){
  for( int i = 0; i < 12; i++){
    strip.setPixelColor(striche[i], stricheColor);
  }
}

void clearStrip(){
  for(uint8_t i = 0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
}

void timerIsr(){
  Sekunden += 1;
  if(Sekunden > 59){
    Sekunden = 0;
    Minuten += 1;
    if(Minuten > 59){
      Minuten = 0;
      Stunden +=1;
      if( Stunden > 23) Stunden = 0;
    }
  }
  updateTime();
}
