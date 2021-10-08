// Fire2012 with programmable Color Palette
// This project uses the Fire2012 example from the FastLED Example Library
// I have removed many of the comments and examples for readability, but you can find
// the original example here: 
// https://github.com/FastLED/FastLED/blob/master/examples/Fire2012/Fire2012.ino
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY

#include <ezButton.h>
#include <FastLED.h>

#define LED_PIN     6
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    12

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

CRGBPalette16 gPal;
ezButton modeButton(4);
ezButton powerButton(4);
bool isPoweredOn = false;
bool modeButtonPressed = false;
bool powerButtonPressed = false;

void setup() {
  Serial.begin(9600);
  delay(3000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.clear();
  FastLED.show();
  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  gPal = HeatColors_p;
  
  modeButton.setDebounceTime(100);
  powerButton.setDebounceTime(3000);

}

void loop()
{
  powerButton.loop();
  modeButton.loop();

  if(modeButton.isReleased()){
    modeButtonPressed = false;
    Serial.println("Released Mode Button");
    powerButtonPressed = false;
    Serial.println("Released Power Button");
  }
  
  if(!isPoweredOn){
    if(modeButton.isPressed() && !powerButtonPressed){
       isPoweredOn = true;
       modeButtonPressed = true;
       Serial.println("Pressed Mode Button");
       powerButton.resetCount();
    }
    FastLED.clear();
    FastLED.show();
  }else{
    if(powerButton.isPressed()){
       isPoweredOn = false;
       powerButtonPressed = true;
       Serial.println("Pressed Power Button");
    }
    
    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy( random());
  
    Fire2012WithPalette(); // run simulation frame, using palette colors
    
    FastLED.show(); // display this frame
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
 
}

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}
