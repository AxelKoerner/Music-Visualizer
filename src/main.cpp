//-----------------------------------------------INITIALIZATION-----------------------------------------------//

#include <Arduino.h>
#include <FastLED.h> //using FastLED libary to handle assembler control of the LEDs 
#define STROBE        4
#define RESET         5
#define DC_One        A0
#define DC_Two        A1
#define LED_PIN       6
#define NUMB_LEDS     120
#define BRIGHTNESS    100
#define LED_TYPE      WS2811
#define COLOR_ORDER   GRB

int leftFreq[7];
int rightFreq[7];
CRGB leds[NUMB_LEDS];

//-----------------------------------------------METHODS-----------------------------------------------//

void clearLEDs() {
  for(int i=0; i < NUMB_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
}

void readICs() {
  digitalWrite(RESET,HIGH);
  digitalWrite(RESET,LOW);
  
  for(int i=0; i < 7; i++) {
    digitalWrite(STROBE, LOW);
    delayMicroseconds(10);
    leftFreq[i] = analogRead(A0);
    rightFreq[i] = analogRead(A1);
    digitalWrite(STROBE, HIGH);
  } 
}


//-----------------------------------------------SETUP-----------------------------------------------//

void setup() {
  pinMode(DC_One, INPUT);     //using A0 and A1 as inputs to read out the 7 frequency bands 
  pinMode(DC_Two, INPUT);     //from the Sparkfun Spectum Shield. A0 is the DC analog output 
                              //from the first IC for the left audio channel, A1 is the output
                              //for the second IC for the right audio channel
  pinMode(STROBE, OUTPUT);    //pin 4 is the Strobe pin, it cycles through each frequency channel once activated
  pinMode(RESET, OUTPUT);     //pin 5 is the reset pin for out Sparfun Board
  digitalWrite(STROBE, LOW);  //we write Low to the strobe pin so that is doesnt cycle when we start our program
  digitalWrite(RESET, HIGH);  //reset pin to high, resets entire multiplexer 

  delay(3000);
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUMB_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  clearLEDs();

}


//-----------------------------------------------EVENT LOOP-----------------------------------------------//

void loop() {
  

}
