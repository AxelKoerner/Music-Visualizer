//-----------------------------------------------INITIALIZATION-----------------------------------------------//

#include <Arduino.h>
#include <FastLED.h>                                   //using FastLED libary to handle assembler control of the LEDs 
#define STROBE        4
#define RESET         5
#define DC_One        A0
#define DC_Two        A1
#define LED_PIN       6
#define NUMB_LEDS     120
#define BRIGHTNESS    100
#define LED_TYPE      WS2811
#define COLOR_ORDER   GRB
#define THRESHOLD     80

int leftAudio[7];
int rightAudio[7];
int frequency;
int frequencyRange[7];
int mid = NUMB_LEDS / 2;
int wheelPos = 255;
double decayRatio = BRIGHTNESS / 50;  //bad
uint8_t hue = 0;                                      //uint8 type because it has a range to 255 and if we increment this variable over 255 it jumps back to 0, usefull for rainbow effect
int brightness = 100;
static int counter;


CRGB decay = CRGB(decayRatio, decayRatio, decayRatio);
CRGB blue = CRGB(0, 0, 100);
CRGB red = CRGB(150, 0, 0);
CRGB green = CRGB(0, 150, 0);
CRGB yellow = CRGB(150, 150, 0);
CRGB black = CRGB(0, 0, 0);
CRGB leds[NUMB_LEDS];
CRGB randomColourGenerater;
static int randomColour = 0;
 

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
    leftAudio[i] = analogRead(A0);
    rightAudio[i] = analogRead(A1);
    digitalWrite(STROBE, HIGH);
  } 
}

CRGB Scroll(int pos) {
  pos = abs(pos);
  CRGB color (0,0,0);
  if(pos < 85) {
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = 255 - color.r;
  } else if(pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = 255 - color.g;
    color.b = 0;
  } else if(pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = 255 - color.b;
    color.r = 1;
  }
  return color;
}


void frequencyGetterBand0() {
  frequency = (leftAudio[0] + rightAudio[0]) / 2;
}

void frequencyGetterBand1() {
  frequency = (leftAudio[1] + rightAudio[1]) / 2;
}

void frequencyGetterBandLow() {
  frequency = (leftAudio[0] + rightAudio[0] + leftAudio[1] + rightAudio[1])/ 4;
}

CRGB randomColourChange() {
  switch (randomColour) {
    case 0:
        randomColourGenerater = red;
        break;
    case 1:
        randomColourGenerater = blue;
        break; 
    case 2:
        randomColourGenerater = yellow;
        break;
    case 3:
        randomColourGenerater = green;
        break;  
    default:
        randomColourGenerater = black;
        break;
  }
  return randomColourGenerater;
}
//-----------------------------------------------VISUALIZER MODES-----------------------------------------------//

void SingleBand0RedBlue() {                                               //1) lowest frequency band, single direction from the start of the led strip, Red and Blue
  readICs();  
  frequencyGetterBand1();                            
  for(int i = 0; i < NUMB_LEDS; i++) {
    if (i < (frequency - THRESHOLD)) {
      leds[i] = CRGB(BRIGHTNESS, 0,BRIGHTNESS);
    }
    else
      leds[i] = leds[i] - decay;
      //fadeToBlackBy(leds, NUMB_LEDS, 1);     
  }
  FastLED.show(); 
}


void fadeToBlackTester() {
  readICs();  
  frequencyGetterBand1();                            
  for(int i = 0; i < NUMB_LEDS; i++) {
    if (i < (frequency - THRESHOLD)) {
      leds[i] = yellow;
    }
  }
  FastLED.show();
  EVERY_N_MILLISECONDS(5) {
      fadeToBlackBy(leds, NUMB_LEDS, 5);
    }
}

  
void SingleBand1Piping() {
  readICs();
  frequencyGetterBand1();
  for(int i = NUMB_LEDS - 1; i >= 3; i--) {
    leds[i] = leds[i - 3]; 
    leds[i-3].fadeToBlackBy(50); 
  }
  if(frequency > THRESHOLD) {
    for(int i = 0; i < 3; i++) {
      leds[i] = randomColourChange();
    }
  }  
  if(counter == 120) 
    counter = 0;
  FastLED.show();
  delay(5);
  EVERY_N_MILLISECONDS(5) {
    randomColour ++;
    if (randomColour > 3);
      randomColour = 0;
  }
}


void AllBands() {                                                     // checks all frequency bands and displays them on the leds with different colors when they pass over the threshold
  readICs();
  for(int i = 0;i < 7; i++) {
    frequencyRange[i] = (leftAudio[i] + rightAudio[i]) / 2;
    if(frequencyRange[i] > THRESHOLD) {
      leds[i] = CRGB(51, 255, 153);
    }
    else 
      leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}


void Rainbow() {                                                     // Rainbow on whole led strip, changes color by increasing hue variable
   for(int i = 0; i < NUMB_LEDS; i++) {
      leds[i] = CHSV(hue + (i * 10), 255, brightness);
    }
  EVERY_N_MILLISECONDS(15) {
    hue++;
  }
  FastLED.show();
}


void RainbowReactiveBand0() {
  readICs();
  frequencyGetterBand1();
   if(frequency > THRESHOLD) {
    for(int i = 0; i < NUMB_LEDS; i++) {
       leds[i] = CHSV(hue + (i * 10), 255, 200);
       hue++;
    }
   }
   EVERY_N_MILLISECONDS(15) {
      fadeToBlackBy(leds, NUMB_LEDS, 1);
      hue++;
    }
  FastLED.show();
}


void Random() {
  EVERY_N_MILLISECONDS(100) {
     for(int i = 0; i < NUMB_LEDS; i++) {
      leds[i] = CHSV(hue + (i * 10), random8(), random8(50, 150));
    }
    EVERY_N_MILLISECONDS(15) {
    hue++;
    }
  FastLED.show();
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
 //SingleBand0RedBlue();
 SingleBand1Piping();
 //AllBands();
 //Rainbow();
 //RainbowReactiveBand0();
 //Random();
 //fadeToBlackTester();
}