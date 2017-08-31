#pragma once

#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(x)      Serial.print (x)
#define DEBUG_PRINTDEC(x,DEC) Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)    Serial.println (x)
#define DEBUG_PRINTLNDEC(x,DEC) Serial.println (x, DEC)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x,DEC)
#define DEBUG_PRINTLN(x) 
#define DEBUG_PRINTLNDEC(x,DEC)
#endif

#define NEOPIXELPIN    D7   //neopixel gpio pin
#define NUMPIXELS      8  // number of pixels in ring

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

#define DEGREESPERSTEP 15;
struct Colour { int r;int g; int b; };

Colour _oldColour = { 255, 255, 255 };
Colour _currentColour = { 255, 255, 255 };

Colour colourRanges[] = {
  { 220, 220, 220 },
  { 0,   0,   255 },
  {   0, 255,   0 },
	{ 215, 215,   0 },
  { 255,   0,   0 }
};

int _brightness = 100;
bool _isDisplayOn = true;

Colour findColourInRange(float PercentageVal) {
  int __lb = 0;
  int __ub = 0;
  int __lbT = 0;
  int __ubT = 0;

  if (PercentageVal < 0)   { PercentageVal = 0;  }
  if (PercentageVal > 40)  { PercentageVal =  40;}

  if (PercentageVal <= 0)                       { __lb = 0;  __ub = 1; __lbT = 0; __ubT = 0; }
  if (PercentageVal > 0 && PercentageVal < 25)  { __lb = 1;  __ub = 2; __lbT = 1;   __ubT = 24; }
  if (PercentageVal >= 25 && PercentageVal < 40){ __lb = 1;  __ub = 2; __lbT = 25;   __ubT = 39; }
  if (PercentageVal >= 40)                      { __lb = 2;  __ub = 3; __lbT = 40;  __ubT = 40;  }


  DEBUG_PRINT("LowerBound: "); DEBUG_PRINTLN(__lb);
  DEBUG_PRINT("UpperBound: "); DEBUG_PRINTLN(__ub);

  float __step1 = (PercentageVal - __lbT);

  Colour __newCol;
  float __r = (colourRanges[__ub].r - colourRanges[__lb].r) / DEGREESPERSTEP;
  float __g = (colourRanges[__ub].g - colourRanges[__lb].g) / DEGREESPERSTEP;
  float __b = (colourRanges[__ub].b - colourRanges[__lb].b) / DEGREESPERSTEP;

  __newCol.r = ( __r * __step1) + colourRanges[__lb].r;
  __newCol.g = ( __g * __step1) + colourRanges[__lb].g;
  __newCol.b = ( __b * __step1) + colourRanges[__lb].b;

  return __newCol;
}

void fade(Colour fromColour, Colour toColour) {

	int n = 200; //#steps
	int Rnew = 0, Gnew = 0, Bnew = 0;

	for (int i = 0; i <= n; i++)
	{
		Rnew = fromColour.r + (toColour.r - fromColour.r) * i / n;
		Gnew = fromColour.g + (toColour.g - fromColour.g) * i / n;
		Bnew = fromColour.b + (toColour.b - fromColour.b) * i / n;

		Rnew = (Rnew*_brightness) / 100;
		Gnew = (Gnew*_brightness) / 100;
		Bnew = (Bnew*_brightness) / 100;

		for (int j = 0; j < NUMPIXELS; j++) {
			// Set pixel color here.
			pixels.setPixelColor(j, pixels.Color(Rnew, Gnew, Bnew));
			pixels.show();
		}
		delay(10);
	}
}

void OutputColour(Colour _col) {
	DEBUG_PRINT("R: "); DEBUG_PRINTLN(_col.r);
	DEBUG_PRINT("G: "); DEBUG_PRINTLN(_col.g);
	DEBUG_PRINT("B: "); DEBUG_PRINTLN(_col.b);
}

void clearpixels() {
  for( int i = 0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i, 0x000000); pixels.show();
  }
}

uint32_t dimColor(uint32_t color, uint8_t width) {
   return (((color&0xFF0000)/width)&0xFF0000) + (((color&0x00FF00)/width)&0x00FF00) + (((color&0x0000FF)/width)&0x0000FF);
}

// Using a counter and for() loop, input a value 0 to 251 to get a color value.
// The colors transition like: red - org - ylw - grn - cyn - blue - vio - mag - back to red.
// Entering 255 will give you white, if you need it.
uint32_t colorWheel(byte WheelPos) {
  byte state = WheelPos / 21;
  switch(state) {
    case 0: return pixels.Color(255, 0, 255 - ((((WheelPos % 21) + 1) * 6) + 127)); break;
    case 1: return pixels.Color(255, ((WheelPos % 21) + 1) * 6, 0); break;
    case 2: return pixels.Color(255, (((WheelPos % 21) + 1) * 6) + 127, 0); break;
    case 3: return pixels.Color(255 - (((WheelPos % 21) + 1) * 6), 255, 0); break;
    case 4: return pixels.Color(255 - (((WheelPos % 21) + 1) * 6) + 127, 255, 0); break;
    case 5: return pixels.Color(0, 255, ((WheelPos % 21) + 1) * 6); break;
    case 6: return pixels.Color(0, 255, (((WheelPos % 21) + 1) * 6) + 127); break;
    case 7: return pixels.Color(0, 255 - (((WheelPos % 21) + 1) * 6), 255); break;
    case 8: return pixels.Color(0, 255 - ((((WheelPos % 21) + 1) * 6) + 127), 255); break;
    case 9: return pixels.Color(((WheelPos % 21) + 1) * 6, 0, 255); break;
    case 10: return pixels.Color((((WheelPos % 21) + 1) * 6) + 127, 0, 255); break;
    case 11: return pixels.Color(255, 0, 255 - (((WheelPos % 21) + 1) * 6)); break;
    default: return pixels.Color(0, 0, 0); break;
  }
}

// Cycles - one cycle is scanning through all pixels left then right (or right then left)
// Speed - how fast one cycle is (32 with 16 pixels is default KnightRider speed)
// Width - how wide the trail effect is on the fading out LEDs.  The original display used
//         light bulbs, so they have a persistance when turning off.  This creates a trail.
//         Effective range is 2 - 8, 4 is default for 16 pixels.  Play with this.
// Color - 32-bit packed RGB color value.  All pixels will be this color.
// knightRider(cycles, speed, width, color);
void knightRider(uint16_t cycles, uint16_t speed, uint8_t width, uint32_t color) {
  uint32_t old_val[NUMPIXELS]; // up to 256 lights!
  // Larson time baby!
  for(int i = 0; i < cycles; i++){
    for (int count = 1; count<NUMPIXELS; count++) {
      pixels.setPixelColor(count, color);
      old_val[count] = color;
      for(int x = count; x>0; x--) {
        old_val[x-1] = dimColor(old_val[x-1], width);
        pixels.setPixelColor(x-1, old_val[x-1]); 
      }
      pixels.show();
      delay(speed);
    }
    for (int count = NUMPIXELS-1; count>=0; count--) {
      pixels.setPixelColor(count, color);
      old_val[count] = color;
      for(int x = count; x<=NUMPIXELS ;x++) {
        old_val[x-1] = dimColor(old_val[x-1], width);
        pixels.setPixelColor(x+1, old_val[x+1]);
      }
      pixels.show();
      delay(speed);
    }
  }
}

void pixelSetBrightness(int brightPercentage) {
  
  //pixels.setBrightness(__brightness) is too lossy
  float __newR, __newG, __newB;
  __newR = _currentColour.r*brightPercentage / 100;
  __newG = _currentColour.g*brightPercentage / 100;
  __newB = _currentColour.b*brightPercentage / 100;
  
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color((int)__newR, (int)__newG, (int)__newB)); // set rgb values
  }
  pixels.show();
}

///////////////////// loading neopixel //////////////////////////////////
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return (pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0) * 3);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return (pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3) * .3);
  }
  else {
    WheelPos -= 170;
    return (pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3) * .3);
  }
}

void loading_colors() {
  DEBUG_PRINTLN("Loading Colours");
  knightRider(3, 32, 2, 0xFFFFFF);
  knightRider(3, 32, 2, 0xFFFFFF);
}

void loadingWheel(){
  uint16_t i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 200));
    }   
    pixels.setBrightness(_brightness);
    pixels.show();
    delay(10);

  }
}

///////////////////// clear neopixel //////////////////////////////////
void clearcolors(bool _delay) {
  DEBUG_PRINTLN("Clear Colours");
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // set rgb values
    pixels.show(); // This sends the updated pixel color to the hardware.
    if (_delay) delay(100);
  }
}

// Pause = delay between transitions
// Steps = number of steps
// R, G, B = "Full-on" RGB values
void breathe2(int pause, int steps, byte R, byte G, byte B) {

  int tmpR, tmpG, tmpB;         // Temp values
  
  // Fade down
  for (int s = steps; s > 0; s--) {
    tmpR = (R * s * _brightness) / steps / 100;     // Multiply first to avoid truncation errors  
    tmpG = (G * s * _brightness) / steps / 100;
    tmpB = (B * s * _brightness) / steps / 100;

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, tmpR, tmpG, tmpB);
    }
    
    pixels.show();
    delay(pause);
  }

  // Fade back up
  for (int s = 1; s <= steps; s++) {
    tmpR = (R * s * _brightness) / steps / 100;     // Multiply first to avoid truncation errors  
    tmpG = (G * s * _brightness) / steps / 100;
    tmpB = (B * s * _brightness) / steps / 100;

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, tmpR, tmpG, tmpB);
    }

    pixels.show();
    delay(pause);
  }
}
