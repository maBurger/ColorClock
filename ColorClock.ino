#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define LICHTSENSOR A3

uint8_t Sekunden = 50, Minuten = 2, Stunden = 1;
uint8_t striche[12] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
uint8_t isrCounter = 0; 
uint8_t minBrightness = 30;

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
  pinMode(LICHTSENSOR, INPUT);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Sek_Color = strip.Color(0, 255, 0); // Green
  Min_Color = strip.Color(0, 0, 255); // Blue
  Std_Color = strip.Color(255, 0, 0); // Red
  stricheColor = strip.Color(20, 20, 20);
  
  updateTime();
  
  Timer1.initialize(250000); // set a timer 250 milli seconds 
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
  
  //debugging
  //Serial.begin(9600);
}

void loop(){
}

void updateTime() {
  uint32_t help_Color;
  
  clearStrip();
  setHours();
  
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

void updateBrightness() {
  uint16_t lightRead;
  
  lightRead = analogRead(LICHTSENSOR);
  //Serial.print(lightRead);
  //Serial.print("  ");
  lightRead = map(lightRead, 0, 1023, 255, minBrightness);
  //Serial.print(lightRead);
  //Serial.print("  ");
  strip.setBrightness(constrain(lightRead, minBrightness, 255));
  //Serial.println(constrain(lightRead, minBrightness, 255));
}

void setHours(){
  for( int i = 0; i < 12; i++){
    strip.setPixelColor(striche[i], stricheColor);
  }
}

void minRun1(){
  for(uint8_t i = 0; i < strip.numPixels(); i++ ){
    strip.setPixelColor(i, Sek_Color);
    strip.show();
    delay(5);
  }
}

void minRun2(){
  for(uint8_t i = 0; i < 30; i++ ){
    strip.setPixelColor(i, Sek_Color);
    strip.setPixelColor(59-i, Sek_Color);
    strip.show();
    delay(5);
  }
}

void clearStrip(){
  for(uint8_t i = 0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
}

void timerIsr(){
  switch (isrCounter) {
    case 0:
      updateBrightness();
      break;
    case 1:
      updateBrightness();
      break;
    case 2:
      updateBrightness();
      break;
    case 3:
      updateBrightness();
      
      //
      Sekunden += 1;
      if(Sekunden > 59){
        Sekunden = 0;
        Minuten += 1;
        minRun1();
        if(Minuten > 59){
          Minuten = 0;
          Stunden +=1;
          if( Stunden > 23) Stunden = 0;
        }
      }
      updateTime();
      break;
  }
  isrCounter += 1;
  if( isrCounter == 4 ) isrCounter = 0;
}
