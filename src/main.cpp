//----------------------------------------------------------------------------------------------INITIALIZATION----------------------------------------------------------------------------------------------//

                                                         // global variables defining pins on the microcontroller, different customizations like brightness etc. and the led type you are using 

#include <Arduino.h>
#include <FastLED.h>                                     // using FastLED libary to handle assembler control of the LEDs 
#include <IRremote.h>

#define STROBE        4                                  // strobe pin to cycle through the 7 different frequency bands, see https://learn.sparkfun.com/tutorials/spectrum-shield-hookup-guide-v2 for a hookup guide 
#define RESET         5                                  // reset pin needs to be set to low, enables the strobe pin. reset pin to high, to reset entire multiplexer
#define DC_One        A0                                 // digital 
#define DC_Two        A1                                 //
#define LED_PIN       6                                  // data output pin   
#define NUMB_LEDS     114                                // number of leds on the led strip
#define BRIGHTNESS    50                                 // brightness of the leds on the strip
#define LED_TYPE      WS2811                             // led type u connect to the microcontroller
#define COLOR_ORDER   GRB                                // 
#define IR_PIN        8                                  //Pin for data from the IR receiver
#define DELAY         5                                 // global delay variable used to delay after a methode has been run through once 
#define DECAY         170                                // decay variable used to dimm down the leds a certaint amount 
#define POWERINMILLIWATTS 300                            // global variable used to prevent the leds from drawing too much power

int VISUALIZER_MODE = 0;
uint32_t IR_result;
int THRESHOLD = 90;                                       // used to set a threshold in reactive methods after the function will for example light up leds on the strip, the treshold should be ajusted depending on what band we are reading from
                                                          // for example, low band like 1 often peak at around 100 - 120, high bands only peak at around 50 - 70 

int leftAudio[7];                                        // used to store the frquency of 7 bands of the first IC module 
int rightAudio[7];                                       // used to store the frquency of 7 bands of the second IC module 
int frequency;                                           // variable to safe the frequency of a specific band                                      
int frequencyRange[7];                                   //
int mid = NUMB_LEDS / 2;
uint8_t hue = 0;                                        // uint8 type because it has a range to 255 and if we increment this variable over 255 it jumps back to 0, usefull for rainbow effects
int brightness = 100;


IRrecv irrrecv(IR_PIN);                                 //initialize the ir receiver
decode_results results;


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

void frequencyAllBands() {
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

CRGB ColorFraction(CRGB colorIn, float fraction) {
  fraction = min(1.0f, fraction);
  return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

void DrawPixels(float fPos, float count, CRGB color) {
  float availFirstPixel = 1.0f - (fPos - (long)(fPos));
  float amtFirstPixel = min(availFirstPixel, count);
  float remaining = min(count, FastLED.size()-fPos);
  int iPos = fPos;

  if(remaining > 0.0f) {
    FastLED.leds()[iPos++] += ColorFraction(color, amtFirstPixel);
    remaining -= amtFirstPixel;
  }

  while(remaining > 1.0f) {
    FastLED.leds()[iPos++] += color;
    remaining--;
  }

  if(remaining > 0.0f) {
    FastLED.leds()[iPos] += ColorFraction(color,remaining);
  }
}




//##############################################################################################-VISUALIZER MODES-#############################################################################################//

//----------------------------------------------------------------------------------------------REACTIVE MUSIC VISUALIZER----------------------------------------------------------------------------------------------//

void SingleBand() {                                               // lights up leds based on the frequency level, fades them to black if amplitude isnt reached anymore
  readICs();  
  frequencySingleBand(0);                            
  for(int i = 0; i < NUMB_LEDS; i++) {
    if (i < (frequency - THRESHOLD)) {
      leds[i] = CRGB(BRIGHTNESS, 0,BRIGHTNESS);
    }
    else
      fadeToBlackBy(leds, NUMB_LEDS, 1);     
  }
  FastLED.show(); 
}


void ReactiveBand1() {                                          // reacts on band 1, if the frequency is higher than the threshold this mode creates a comet that changes colour 

  readICs();
  frequencySingleBand(1);

  const int cometSize = 4;
  const int deltaHue = 4;
  const byte fade = 128;
  const int cometTail = cometSize * 2;
  static byte hue = HUE_RED;

  hue += deltaHue;

  for(int i = NUMB_LEDS - 1; i > 0; i--) {
    leds[i] = leds[i - 1];
  }

  if(frequency > THRESHOLD) {
    for(int j = 0; j < cometSize + cometTail; j++) {
    leds[j].setHue(hue);
    hue += deltaHue;
    }

    //for(int k = 0; k <= cometTail; k++) {if(random(2) == 1) {leds[k].fadeToBlackBy(fade);}}
   // for(int k = 0; k <= cometTail; k++) {if(random(2) == 1) {leds[k].fadeToBlackBy(fade);}}
 

    for(int i = cometTail; i >= 0; i--) {
      if(random(i+1) == 0)
        leds[i].fadeToBlackBy(fade);
      if(random(i+1) == 1)
        leds[i].fadeToBlackBy(fade);
    }

  }
  else {
      leds[0] = black;
   }
  FastLED.show();
  delay(1);
}



void smoothAnim() {
  static float scroll = 0.0f;
  scroll += 0.1f;
  if(scroll > 5.0)
    scroll -=5.0;

  for(float i = scroll; i < NUMB_LEDS; i+=5) {
    DrawPixels(i, 3, CRGB::Blue);
  }

  fadeToBlackBy(leds, NUMB_LEDS, 155);
  FastLED.show();

}



void sides() {                                        // 4 sides that each represent a frequency band, activate when over the threshold 
  static const int sides = 4;
  readICs();
  for(int i = 0; i <= 3; i++) {
    frequencySingleBand(i);
    frequencyRange[i] = frequency;
  }

  const byte fade = 5;
  const int sidesNumb = (int) NUMB_LEDS / sides;            //leds per Side


  for(int i = 0; i <= 3; i++) {
    if(frequencyRange[i] > THRESHOLD) {
      switch (i) {
        case 0: for(int k = 0; k < sidesNumb ; k++ ) {leds[k].setRGB(255,0,0);} break;
        case 1: for(int k = sidesNumb; k < sidesNumb * 2; k++ ) {leds[k].setRGB(0,0,255);} break;
        case 2: for(int k = sidesNumb*2; k < sidesNumb * 3; k++ ) {leds[k].setRGB(0,255,0);} break;
        case 3: for(int k = sidesNumb*3; k < sidesNumb * 4; k++ ) {leds[k].setRGB(255,255,255);} break;
        default: break;
      }
    }
  }




  for(int j = NUMB_LEDS - 1; j >= 0; j--) {
    leds[j].fadeToBlackBy(fade);
  }

  FastLED.show();
  delay(10);
}






void ledsOnPerFreq() {
  readICs();
  frequencySingleBand(1);

  // function to map the range of values to the range of leds
  int ledFreq = NUMB_LEDS/THRESHOLD;
  int ledsOn = (frequency-THRESHOLD) * ledFreq;
  static int fade = 20;
  static byte hue = HUE_BLUE;
  static byte deltaHue = 4;

  for(int i = 0; i <= ledsOn; i++) {
    leds[i].setHue(hue);
  }

  

  //fadeToBlackBy(leds, NUMB_LEDS, fade);
  EVERY_N_MILLISECONDS(10) {
    for(int i = NUMB_LEDS-1; i >= 0; i--) {
    leds[i].fadeToBlackBy(fade);
  }
  }
  
  hue += deltaHue;
  FastLED.show();
  delay(5);
}




void reactiveComet() {
  readICs();
  frequencySingleBand(0);

  //const int cometSize = 4;
  const byte fade = 128;
  static int positions [15];
  const int steps = 1;
  static byte hue = HUE_BLUE;
  static byte deltaHue = 4;
  const int arrayLength = sizeof(positions)/sizeof(int);


  for(int i = 0; i <= arrayLength - 1; i++) {   //checks if positions are out led strip scope 
    if(positions[i] > NUMB_LEDS - 2)
      positions[i] = -1;
  }
  
  if(frequency > THRESHOLD) {                     //initiates new positions for comets in array
    for(int i = 0; i <= arrayLength - 1; i++) {
      if(positions[i] == -1) {
         positions[i] = 0;
        break;
      }
    }
  }

  for(int i = 1; i <= arrayLength - 1; i++) {                                           //draws on led strip
    if(positions[i] != -1) {
      leds[positions[i]].setHue(hue);
      leds[(positions[i] + steps)].setHue(hue);
      positions[i] += steps;
    }
  }

  for(int k = NUMB_LEDS; k >= 0; k--) {
    if(random(2) == 1)
      leds[k].fadeToBlackBy(fade);
  }

  FastLED.show();
  delay(DELAY);
  hue += deltaHue;
}


void CometChangeDirection() {                                      //comet that changes directions when the frequency is higher than the threshold 
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
  delay(1);
}


const void SingleBand1Piping() {                                  // different type of methode used to visualize, pipes the leds through the strip when frequency is higher than the threshold                                      
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


void doubleRainbow() {                                             //splits the strip in two parts and lights up leds based on the frequency level, rainbow colour through adding up to the hue variable
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

void Twinkle() {                                                              // lights up random leds on the strip with random colours
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
void Comet() {                                                                // Comet effect, cant be made reactive because the nature of the implementation
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


void Random() {                                                       // lights up random colours with a different methode to choose the colour
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

void shutterLight() {
  for(int i = 0; i < NUMB_LEDS; i++) {
    leds[i].setRGB(255,255,255);
  }
  FastLED.show();

  fadeToBlackBy(leds,NUMB_LEDS,255);
  
  FastLED.show();
  delay(50);
}


//----------------------------------------------------------------------------------------------SETUP----------------------------------------------------------------------------------------------//

void setup() {
  pinMode(DC_One, INPUT);                                                                          //using A0 and A1 as inputs to read out the 7 frequency bands 
  pinMode(DC_Two, INPUT);                                                                          //from the Sparkfun Spectum Shield. A0 is the DC analog output 
                                                                                                   //from the first IC for the left audio channel, A1 is the output
                                                                                                   //for the second IC for the right audio channel
  pinMode(STROBE, OUTPUT);                                                                         //pin 4 is the Strobe pin, it cycles through each frequency channel once activated
  pinMode(RESET, OUTPUT);                                                                          //pin 5 is the reset pin for out Sparfun Board
  digitalWrite(STROBE, LOW);                                                                       //we write Low to the strobe pin so that is doesnt cycle when we start our program
  digitalWrite(RESET, HIGH);                                                                       //reset pin to high, resets entire multiplexer 

  delay(1000);
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUMB_LEDS).setCorrection(TypicalLEDStrip);       //inititalization of the leds, FastLED methode 
  //FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(ledsSet,NUMB_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  clearLEDs();
  FastLED.setMaxPowerInMilliWatts(POWERINMILLIWATTS);


  Serial.begin(9600);                 //serial communication with 9600 baud
  irrrecv.enableIRIn();               //activate the IR Pin
  //pinMode(IR_PIN, INPUT);             //declare IR Pin as input
  
  

}


//----------------------------------------------------------------------------------------------EVENT LOOP----------------------------------------------------------------------------------------------//

void loop() {                                                 // define methode to run in the event loop 


while (!irrrecv.isIdle());                                    // if not idle, wait till complete
if(IrReceiver.decode()) {
  
  IR_result = IrReceiver.decodedIRData.decodedRawData;

  switch (IR_result) {
    case 4161273600:            //threshold down
      Serial.println("threshold down");
      if(THRESHOLD >= 50) 
        THRESHOLD--;
      break;

    case 3927310080:           //threshhold up
    Serial.println("threshold up");
      if(THRESHOLD <= 130)
        THRESHOLD++;
      break;

    case 3910598400:          //shutter light - 0
      Serial.println("shutter light");
      VISUALIZER_MODE = 0;
      break; 

    case 4077715200:         //reactive Comet - 1
      Serial.println("reactive Comet");
      VISUALIZER_MODE = 1;
      break; 

    case 3877175040:         //ledsOnPerFreq - 2
      Serial.println("leds on per frequency");
      VISUALIZER_MODE = 2;
      break; 

    case 2707357440:         //smoothAnim - 3
      Serial.println("smooth Animation");
      VISUALIZER_MODE = 3;
      break;

    case 4144561920:         //SingleBand - 4
      Serial.println("Single Band");
      VISUALIZER_MODE = 4;
      break;
    
    case 3810328320:         //ReactiveBand1 - 5
      Serial.println("Reactive Band 1");
      VISUALIZER_MODE = 5;
      break;
    
    case 2774204160:         //Sides - 6
      Serial.println("Sides");
      VISUALIZER_MODE = 6;
      break;

    case 3175284480:         //CometChangeDirection - 7
      Serial.println("Comet Change Direction");
      VISUALIZER_MODE = 7;
      break;

    case 2907897600:         //SingleBand1Piping - 8
      Serial.println("Single Band 1 Piping");
      VISUALIZER_MODE = 8;
      break;

    case 3041591040:         //doubleRainbow - 9
      Serial.println("Double Rainbow");
      VISUALIZER_MODE = 9;
      break;

    default:
      break;
  }
  irrrecv.resume();
}

/* von top left to bottom rigth all hex values from the ir remote 
  BA45FF00 B946FF00 B847FF00
  BB44FF00 BF40FF00 BC43FF00
  F807FF00 EA15FF00 F609FF00
  E916FF00 E619FF00 F20DFF00
  F30CFF00 E718FF00 A15EFF00
  F708FF00 E31CFF00 A55AFF00
  BD42FF00 AD52FF00 B54AFF00
*/ 

switch (VISUALIZER_MODE) {
  case 0:
    shutterLight();
    break;

  case 1:
    reactiveComet();
    break;

  case 2:
    ledsOnPerFreq();
    break;

  case 3:
    smoothAnim();
    break;

  case 4:
    SingleBand();   //todo
    break;
    
  case 5:
    ReactiveBand1();
    break;  

  case 6:
    sides();
    break;

  case 7:
    CometChangeDirection();
    break;

  case 8:
    SingleBand1Piping();
    break;

  case 9:
    doubleRainbow();
    break;

  default:
    shutterLight();
}

/*
shutterLight()
reactiveComet()
ledsOnPerFreq()
smoothAnim()
SingleBand()
ReactiveBand1()
sides()
CometChangeDirection()
SingleBand1Piping()
doubleRainbow()
*/

}
