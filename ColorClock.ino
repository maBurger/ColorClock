#include <Wire.h>
#include <RTClib.h>
#include <ClickButton.h>
#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define LICHTSENSOR A3
#define BUTTONPIN 4

uint8_t Sekunden = 50, Minuten = 2, Stunden = 1;
uint8_t setSek = 0, setMin = 0, setH = 0, setCount = 0;
uint8_t striche[12] = { 
  0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
uint8_t isrCounter = 0;
uint8_t isrBlinkColor = 0;
uint8_t minBrightness = 30;

#define TIME_RUN_MODE 10
#define TIME_UPDATE_MODE 11
#define TIME_RTC_READ 12

#define MENU_MODE 20
#define MENU_SET_MODE 24
#define MENU_SET_H   0
#define MENU_SET_MIN 1
#define MENU_SET_SEC 2
#define MENU_SET_RTC 3

#define RAINBOW_MODE 30

uint8_t displayStatus = TIME_UPDATE_MODE;
uint8_t menuSetStatus = MENU_SET_H;

uint8_t menuItem = 0;
uint8_t menuArray[4][3] = {
  {59, 0, 1},
  {14, 15, 16},
  {29, 30, 31},
  {44, 45, 46}
};


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);
uint32_t Sek_Color, Min_Color, Std_Color, stricheColor, menuColor, menuNotSelColor;

// Click Button Initalize
ClickButton button1(BUTTONPIN, LOW, CLICKBTN_PULLUP);

RTC_DS1307 RTC;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  pinMode(LICHTSENSOR, INPUT);
  //debugging
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();

  DateTime now = RTC.now();
  Stunden = now.hour();
  Minuten = now.minute();
  Sekunden = now.second();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Sek_Color       = strip.Color(0, 255, 0); // Green
  Min_Color       = strip.Color(0, 0, 255); // Blue
  Std_Color       = strip.Color(255, 0, 0); // Red
  stricheColor    = strip.Color(20, 20, 20);
  menuColor       = strip.Color(0, 255, 0);
  menuNotSelColor = strip.Color(0, 0, 88);

  updateTime();

  Timer1.initialize(250000); // set a timer 250 milli seconds Timer will fire 4 times in a second
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

  // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  button1.debounceTime   = 20;   // Debounce timer in ms
  button1.multiclickTime = 250;  // Time limit for multi clicks
  button1.longClickTime  = 1000; // time until "held-down clicks" register
}

void loop(){
  button1.Update();

  if( button1.clicks != 0 ){
    // we need to build up a state machine
    switch( displayStatus ){
      case TIME_RUN_MODE:
        // if( button1.clicks == 1 ) displayStatus = TIME_MODE;
        if( button1.clicks == 2 ) displayStatus = RAINBOW_MODE;
        if( button1.clicks == 3 ) displayStatus = TIME_RTC_READ;
        if( button1.clicks == -3 ) displayStatus = MENU_MODE;
        break;
      case MENU_MODE:
        // one short click --> goto next menu item
        if( button1.clicks == 1 ){
          menuItem++;
          if( menuItem > 3) menuItem = 0;
        }
        
        // on long click --> enter the menu item
        if( button1.clicks == -1 ) {
          switch(menuItem){
            case 0:
              displayStatus = TIME_UPDATE_MODE;
              break;
            case 1:
              displayStatus = TIME_UPDATE_MODE;
              break;
            case 2:
              displayStatus = TIME_UPDATE_MODE;
              break;
            case 3:
              displayStatus = MENU_SET_MODE;
              setH = 0;
              setMin = 0;
              setSek = 0;
              break;
          }
        }
        
        // 3 clicks long --> leave the menu mode
        if( button1.clicks == -3 ){
          displayStatus = TIME_UPDATE_MODE;
          menuItem = 0;
        }
        break;
        
        case MENU_SET_MODE:
          // one click move on one step (hour, minutes seconds
          if( button1.clicks > 0 ){
            setCount += button1.clicks;
            if(menuSetStatus == MENU_SET_H){
              if(setCount > 11) setCount = 0;
            } else {
              if(setCount > 59) setCount = 0;
            }
          }
          
          // goto next setting
          if( button1.clicks == - 1 ){
            switch(menuSetStatus){
              case MENU_SET_H:
                menuSetStatus = MENU_SET_MIN;
                setH = setCount;
                setCount = 0;
                break;
              case MENU_SET_MIN:
                menuSetStatus = MENU_SET_SEC;
                setMin = setCount;
                setCount = 0;
                break;
              case MENU_SET_SEC:
                setSek = setCount;
                Serial.print("New Time: ");
                Serial.print(setH, DEC);
                Serial.print(':');
                Serial.print(setMin, DEC);
                Serial.print(':');
                Serial.print(setSek, DEC);
                Serial.println();
                Stunden = setH;
                Minuten = setMin;
                Sekunden = setSek;
                DateTime newTime = DateTime(0,0,0, setH, setMin, setSek);
                RTC.adjust(newTime);
                displayStatus = TIME_UPDATE_MODE;
                break;
            }
          }
          break;
    }
  }
  updateStrip();
}

void updateStrip(){
  DateTime now;
  
  switch( displayStatus ){
    case TIME_RUN_MODE:
      break;
    case TIME_UPDATE_MODE:
      updateTime();
      displayStatus = TIME_RUN_MODE;
      break;
    case TIME_RTC_READ:
      now = RTC.now();
      Stunden = now.hour();
      Minuten = now.minute();
      Sekunden = now.second();
      displayStatus = TIME_RUN_MODE;
      break;
    case RAINBOW_MODE:
      for( int i = 0; i < 2; i++){
        rainbow(10);
      }
      displayStatus = TIME_UPDATE_MODE;
      break;
    case MENU_MODE:
      clearStrip();
      for( int i = 0; i < 60; i++){
        strip.setPixelColor(i, stricheColor);
      }
      for ( int j = 0; j < 4; j++){
        for( int i = 0; i < 3; i++){
          if( j == menuItem) strip.setPixelColor(menuArray[j][i], menuColor * isrBlinkColor);
          else strip.setPixelColor(menuArray[j][i], menuNotSelColor);
        }
      }
      strip.show();
      break;
      
    case MENU_SET_MODE:
      clearStrip();
      for( int i = 0; i < 60; i++){
        strip.setPixelColor(i, stricheColor);
      }
      switch(menuSetStatus){
        case MENU_SET_H:
          strip.setPixelColor(setCount * 5, Std_Color * isrBlinkColor);
          break;
        case MENU_SET_MIN:
          strip.setPixelColor(setCount, Min_Color * isrBlinkColor);
          break;
        case MENU_SET_SEC:
          strip.setPixelColor(setCount, Sek_Color * isrBlinkColor);
          break; 
      }
      strip.show();
      break;
  }
  
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

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void clearStrip(){
  for(uint8_t i = 0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
}

void toggleBlinkColor(){
  if (isrBlinkColor) isrBlinkColor = 0;
  else isrBlinkColor = 1;
}

void timerIsr(){
  switch (isrCounter) {
  case 0:
    updateBrightness();
    toggleBlinkColor();
    break;
  case 1:
    updateBrightness();
    toggleBlinkColor();
    break;
  case 2:
    updateBrightness();
    toggleBlinkColor();
    break;
  case 3:
    updateBrightness();
    toggleBlinkColor();

    //
    Sekunden += 1;
    if(Sekunden > 59){
      Sekunden = 0;
      Minuten += 1;
      if( displayStatus == TIME_RUN_MODE ) minRun1();
      if(Minuten > 59){
        Minuten = 0;
        Stunden +=1;
        if( Stunden > 23) Stunden = 0;
      }
    }
    // only when the clock is in TIME_RUN_MODE the update must be done
    // especially important when in MENU_MODE
    if(displayStatus == TIME_RUN_MODE) displayStatus = TIME_UPDATE_MODE;
    break;
  }
  isrCounter += 1;
  if( isrCounter == 4 ) isrCounter = 0;
}


