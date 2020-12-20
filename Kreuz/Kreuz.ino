#include <DMXSerial.h>
#include <FastLED.h>

// Constants for demo program
#define DATA_PIN 3
#define NUM_LEDS 280
int BRIGHTNESS = 255;

#define FRAMES_PER_SECOND  120

// Define the array of leds
CRGB leds[NUM_LEDS];
// var for worm
int pos0 = 0;
int pos1 = 93;
int pos2 = 186;

int Dmx_In1=0;
int Dmx_In2=0;
int Dmx_In3=0;
int Dmx_In4=0;
int Dmx_In5=0;
int Dmx_In6=0;
int fade=0;

int r[] = {255, 255, 255,   0,   0};
int g[] = {22,  255,   0, 255,   0};
int b[] = {0,   255,   0,   0, 255};
int colorSize = 5;
int colorArrayIndex = 0;

int upperButton  = 8;
bool upperButtonPressed = false;
int middleButton = 7;
bool middleButtonPressed = false;
int lowerButton  = 4;
bool lowerButtonPressed = false;

int programMode = 0;
int programSize = 4;

int COOLING = 77;
int SPARKING = 128;

void worm(int r, int g, int b)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 25);
  //int pos = beatsin16(13,0,NUM_LEDS);
  pos0 = (pos0+1)%NUM_LEDS;
  pos1 = (pos1+1)%NUM_LEDS;
  pos2 = (pos2+1)%NUM_LEDS;
  leds[pos0] += CRGB( r, g, b);
  leds[pos1] += CRGB( r, g, b);
  leds[pos2] += CRGB( r, g, b);
}

void addGlitter( fract8 chanceOfGlitter, int r, int g, int b) 
{
  fadeToBlackBy( leds, NUM_LEDS, 10);
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB(r, g, b);
  }
}

void setup () {
  delay(2000); // sanity delay
  DMXSerial.init(DMXReceiver);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  pinMode(upperButton, INPUT);
  pinMode(middleButton, INPUT);
  pinMode(lowerButton, INPUT);
}

void Fire(int offset, int numToFill, bool gReverseDirection)
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS/2];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < numToFill; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / numToFill) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= numToFill - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < numToFill; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (numToFill + offset - 1) - j;
      } else {
        pixelnumber = j + offset;
      }
      leds[pixelnumber] = color;
    }
}

int i = 0;
void loop() {
  delay(70);
  unsigned long lastPacket = DMXSerial.noDataSince();
  if (lastPacket < 2000) {
    // read recent DMX values and set pwm levels 
    // Turn the LED on, then pause
    Dmx_In1=DMXSerial.read(43); //rot 
    Dmx_In2=DMXSerial.read(44); //gruen 
    Dmx_In3=DMXSerial.read(45); //blau
    Dmx_In4=DMXSerial.read(46); //effekte 1/3 statisch 2/3 laufen 3/3 blinken
    Dmx_In5=DMXSerial.read(47); //orange
    Dmx_In6=DMXSerial.read(48); //Feuer
    if (Dmx_In5 > 25) {
      fade = ((Dmx_In5-25)*100)/230;
      Dmx_In1 = 255 * fade/100;
      Dmx_In2 = 22 * fade/100;
      Dmx_In3 = 0;
    }
    if (Dmx_In4 < 128){
      for (int i=0; i<=NUM_LEDS; i++){
        leds[i] = CRGB(Dmx_In1,Dmx_In2,Dmx_In3);
      }
    } 
    if (Dmx_In4 >128 && Dmx_In4 < 192){
      worm(Dmx_In1,Dmx_In2,Dmx_In3);
    }
    if (Dmx_In4 > 192){
      addGlitter(30, Dmx_In1,Dmx_In2,Dmx_In3);
    }
    if( Dmx_In6 > 128 ){
      Fire(0,140,false);
      Fire(140,140,true);
      FastLED.setBrightness( (Dmx_In6 -128 ) * 2 - 1 );
      FastLED.delay(100 / FRAMES_PER_SECOND);
    } else {
      FastLED.setBrightness(255);
    }
  } else {
    if(( digitalRead(middleButton) == 1)  &&  (middleButtonPressed == false)){
      colorArrayIndex = colorArrayIndex + 1;
      colorArrayIndex = colorArrayIndex % colorSize;
      middleButtonPressed = true;
    }
    if( digitalRead(middleButton) == 0) {
      middleButtonPressed = false;
    }
    if(( digitalRead(lowerButton) == 1 ) && (lowerButtonPressed == false)){
      programMode = programMode + 1;
      programMode = programMode % programSize;
      lowerButtonPressed = true;
    }

    if( digitalRead(lowerButton) == 0){
      lowerButtonPressed = false;
    }
    switch (programMode){
      case 0:
        // default after start
        fill_solid(leds, NUM_LEDS, CRGB(r[colorArrayIndex], g[colorArrayIndex], b[colorArrayIndex]));
        FastLED.delay(100 / FRAMES_PER_SECOND);
        break;

      case 1:
      // worm
      worm(r[colorArrayIndex],g[colorArrayIndex],b[colorArrayIndex]);
      break;

      case 2:
      // glitter
      addGlitter(30, r[colorArrayIndex],g[colorArrayIndex],b[colorArrayIndex]);
      break;

      case 3:
      // fire
      Fire(0,140,false);
      Fire(140,140,true);
      FastLED.setBrightness( 255 );
      FastLED.delay(100 / FRAMES_PER_SECOND);
    }

  }
  FastLED.show();
}
