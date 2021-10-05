//----------------------------------------------------------------------------------------------INITIALIZATION----------------------------------------------------------------------------------------------//

                                                         // global variables defining pins on the microcontroller, different customizations like brightness etc. and the led type you are using 

#include <Arduino.h>
#include <FastLED.h>                                     // using FastLED libary to handle assembler control of the LEDs 
#define STROBE        4                                  // strobe pin to cycle through the 7 different frequency bands, see https://learn.sparkfun.com/tutorials/spectrum-shield-hookup-guide-v2 for a hookup guide 
#define RESET         5                                  // reset pin needs to be set to low, enables the strobe pin. reset pin to high, to reset entire multiplexer
#define DC_One        A0                                 // digital 
#define DC_Two        A1                                 //
#define LED_PIN       6                                  // data output pin   
#define NUMB_LEDS     120                                // number of leds on the led strip
#define BRIGHTNESS    50                                 // brightness of the leds on the strip
#define LED_TYPE      WS2811                             // led type u connect to the microcontroller
#define COLOR_ORDER   GRB                                // 
#define THRESHOLD     100                                // used to set a threshold in reactive methods after the function will for example light up leds on the strip, the treshold should be ajusted depending on what band we are reading from
                                                         // for example, low band like 1 often peak at around 100 - 120, high bands only peak at around 50 - 70 
#define DELAY         10                                 // global delay variable used to delay after a methode has been run through once 
#define DECAY         170                                // decay variable used to dimm down the leds a certaint amount 
#define POWERINMILLIWATTS 300                            // global variable used to prevent the leds from drawing too much power

int leftAudio[7];                                        // used to store the frquency of 7 bands of the first IC module 
int rightAudio[7];                                       // used to store the frquency of 7 bands of the second IC module 
int frequency;                                           // variable to safe the frequency of a specific band                                      
int frequencyRange[7];                                   //
int mid = NUMB_LEDS / 2;
uint8_t hue = 0;                                        // uint8 type because it has a range to 255 and if we increment this variable over 255 it jumps back to 0, usefull for rainbow effects
int brightness = 100;


CRGB blue    =    CRGB(0, 0, 255);                      // some colours and their pre defined CRB values
CRGB red     =    CRGB(255, 0, 0);
CRGB green   =    CRGB(0, 255, 0);
CRGB yellow  =    CRGB(255, 255, 0);
CRGB orange  =    CRGB(255, 128, 0);
CRGB purple  =    CRGB(255, 0, 255);
CRGB pink    =    CRGB(255, 0, 127);
CRGB white   =    CRGB(255, 255, 255);
CRGB black   =    CRGB(0, 0, 0);
CRGB leds[NUMB_LEDS];                                  // led array to cycle through all leds and accessing them individually 
CRGBArray<NUMB_LEDS> ledsSet;


//----------------------------------------------------------------------------------------------METHODS----------------------------------------------------------------------------------------------//

void clearLEDs() {                                    // clears led strip by setting every led to black aka CRGB(0,0,0)
  for(int i=0; i < NUMB_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
}

void readICs() {                                     // reads out the values of the two ICs and writes the values into the two arrays leftAudio and rightAudio
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


void frequencySingleBand(int frequencyBand) {       // methode to get the frequency of a single band specified by the argument 
  frequency = (leftAudio[frequencyBand] + rightAudio[frequencyBand]) / 2;
}

void frequencyLowBands() {                          // methode to get the low Bands aka frquency bands 0 and 1 put together 
  frequency = (leftAudio[0] + rightAudio[0] + leftAudio[1] + rightAudio[1])/ 4;
}

void frequencyGetterAllBands() {
  for(int i = 0; i < 7; i++) {
    frequencyRange[i] = (leftAudio[i] + rightAudio[i]) / 2;
  }
}

static const CRGB RandomColors [11] = 
{
  CRGB::White,
  CRGB::DarkBlue,
  CRGB::SkyBlue,
  CRGB::Green,
  CRGB::Yellow,
  CRGB::Orange,
  CRGB::Red,
  CRGB::Pink,
  CRGB::FloralWhite,
  CRGB::SeaGreen,
  CRGB::PaleVioletRed
};


//##############################################################################################-VISUALIZER MODES-#############################################################################################//

//----------------------------------------------------------------------------------------------REACTIVE MUSIC VISUALIZER----------------------------------------------------------------------------------------------//

void SingleBand() {                                               // 
  readICs();  
  frequencySingleBand(1);                            
  for(int i = 0; i < NUMB_LEDS; i++) {
    if (i < (frequency - THRESHOLD)) {
      leds[i] = CRGB(BRIGHTNESS, 0,BRIGHTNESS);
    }
    else
      fadeToBlackBy(leds, NUMB_LEDS, DECAY);     
  }
  FastLED.show(); 
}


void ReactiveBand1() {

  readICs();
  frequencySingleBand(1);

  const int cometSize = 5;
  const int deltaHue = 4;
  const byte fade = 128;
  const int cometTail = cometSize / 1.5;
  static byte hue = HUE_RED;

  hue += deltaHue;

  for(int i = NUMB_LEDS - 1; i > 0; i--) {
    leds[i] = leds[i - 1];
    if(leds[i] != black) {
      
    }
  }

  if(frequency > THRESHOLD) {
    for(int j = 0; j < cometSize + cometTail; j++) {
    leds[j].setHue(hue);
    hue += deltaHue;
    }

    //for(int k = 0; k <= cometTail; k++) {if(random(2) == 1) {leds[k].fadeToBlackBy(fade);}}
   // for(int k = 0; k <= cometTail; k++) {if(random(2) == 1) {leds[k].fadeToBlackBy(fade);}}

    for(int i = cometTail; i >= 0; i--) {
      if(random(2) == 1)
        leds[i].fadeToBlackBy(fade);
      if(random(2) == 1)
        leds[i].fadeToBlackBy(fade);
    }

  }
  else {
      leds[0] = black;
   }
  FastLED.show();
  delay(10);
}


void CometChangeDirection() {
  readICs();
  frequencySingleBand(1);
 const byte fade = 128;
  const int cometSize = 8;
  const int deltaHue = 4;

  static byte hue = HUE_RED;
  static int direction = 1;
  static int position = 0;

  hue += deltaHue;
  position += direction;

  if(position == (NUMB_LEDS - cometSize) || position == 0 || frequency > THRESHOLD)  {  
    direction *= -1;
  }

  for (int i = 0; i < cometSize; i++)
    leds[position + i].setHue(hue);
  
  for(int i = 0; i < NUMB_LEDS; i++) {
    if(random(2) == 1)
      leds[i] = leds[i].fadeToBlackBy(fade);
  }
  FastLED.show();
  delay(5);
}


const void SingleBand1Piping() {
  readICs();
  frequencySingleBand(1);
  for(int i = NUMB_LEDS - 1; i >= 2; i--) {
    leds[i] = leds[i - 2]; 
    leds[i-2].fadeToBlackBy(DECAY); 
  }
  if(frequency > THRESHOLD) {
    CRGB newColor = RandomColors[random(0, 10)];
    for(int i = 0; i < 2; i++) {
      leds[i] = newColor; 
    }
  }  
  FastLED.show();
  delay(DELAY);
}


void doubleRainbow() {
  readICs();
  frequencySingleBand(1);

  const int deltaHue = 4;
  static byte hue = HUE_AQUA;
  static byte fade = 128;
  byte range;

  if(frequency > THRESHOLD) {
    range = frequency - THRESHOLD;
    for(int i = 0; i < range; i++) {
      leds[mid + i] = hue;
      leds[mid - i] = hue;
      hue += deltaHue;
    }
  }

  EVERY_N_MILLISECONDS(500) {
    leds->fadeToBlackBy(fade);
  }
}
//----------------------------------------------------------------------------------------------NON REACTIVE VISUALIZER----------------------------------------------------------------------------------------------//

void Twinkle() {
  static int passCount = 0;
  passCount++;
  if (passCount == NUMB_LEDS) {
    passCount = 0;
    FastLED.clear(false);
  }
  leds[random(NUMB_LEDS)] = RandomColors[random(10)];
  FastLED.show();
}

// credit to https://github.com/davepl/DavesGarageLEDSeries/blob/master/LED%20Episode%2007/src/comet.h 
void Comet() {
  const byte fade = 128;
  const int cometSize = 5;
  const int deltaHue = 4;

  static byte hue = HUE_RED;
  static int direction = 1;
  static int position = 0;

  hue += deltaHue;
  position += direction;

  if(position == (NUMB_LEDS - cometSize) || position == 0) {  
    direction *= -1;
  }

  for (int i = 0; i < cometSize; i++)
    leds[position + i].setHue(hue);
  
  for(int i = 0; i < NUMB_LEDS; i++) {
    if(random(2) == 1)
      leds[i] = leds[i].fadeToBlackBy(fade);
  }
  FastLED.show();
  delay(10);
  
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
  frequencySingleBand(1);
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



//----------------------------------------------------------------------------------------------SETUP----------------------------------------------------------------------------------------------//

void setup() {
  pinMode(DC_One, INPUT);                                     //using A0 and A1 as inputs to read out the 7 frequency bands 
  pinMode(DC_Two, INPUT);                                     //from the Sparkfun Spectum Shield. A0 is the DC analog output 
                                                              //from the first IC for the left audio channel, A1 is the output
                                                              //for the second IC for the right audio channel
  pinMode(STROBE, OUTPUT);                                    //pin 4 is the Strobe pin, it cycles through each frequency channel once activated
  pinMode(RESET, OUTPUT);                                     //pin 5 is the reset pin for out Sparfun Board
  digitalWrite(STROBE, LOW);                                  //we write Low to the strobe pin so that is doesnt cycle when we start our program
  digitalWrite(RESET, HIGH);                                  //reset pin to high, resets entire multiplexer 

  delay(1000);
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUMB_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(ledsSet,NUMB_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  clearLEDs();
  FastLED.setMaxPowerInMilliWatts(POWERINMILLIWATTS);

}


//----------------------------------------------------------------------------------------------EVENT LOOP----------------------------------------------------------------------------------------------//

void loop() {                                                 // define methode to run in the endless loop 

 //SingleBand1Piping();
 //Rainbow();
 //Random();
 //Twinkle();
 //AllBandsPiping();
//Comet();
//CometChangeDirection();
ReactiveBand1();
//doubleRainbow();
}