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
#define NUMPIXELS      12  // number of pixels in ring

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

#define DEGREESPERSTEP 15;

struct Colour {
	int r;
	int g;
	int b;
};

Colour _oldColour = { 255, 255, 255 };

Colour colourRanges[] = {
  { 220, 220, 220 },
	{   0,  0,  220 },
	{   0, 220,   0 },
	{ 220,   0,   0 }
};

int _brightness = 100;
bool _isDisplayOn = true;

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

Colour findTempColour(float temp) {
	int __lb = 0;
	int __ub = 0;
	int __lbT = 0;
	int __ubT = 0;

	if (temp < -15) { temp = -15; }
	if (temp >  30) { temp =  30; }

	if (temp < 0)				{ __lb = 0;  __ub = 1; __lbT = -15; __ubT = -1;	}
	if (temp >= 0 && temp < 15) { __lb = 1;  __ub = 2; __lbT = 0; __ubT = 14;	}
	if (temp >= 15)				{ __lb = 2;  __ub = 3; __lbT = 15; __ubT = 30;	}


	DEBUG_PRINT("LowerBound: "); DEBUG_PRINTLN(__lb);
	DEBUG_PRINT("UpperBound: "); DEBUG_PRINTLN(__ub);

	float __step1 = (temp - __lbT);

	Colour __newCol;
	float __r = (colourRanges[__ub].r - colourRanges[__lb].r) / DEGREESPERSTEP;
	float __g = (colourRanges[__ub].g - colourRanges[__lb].g) / DEGREESPERSTEP;
	float __b = (colourRanges[__ub].b - colourRanges[__lb].b) / DEGREESPERSTEP;

	__newCol.r = ( __r * __step1) + colourRanges[__lb].r;
	__newCol.g = ( __g * __step1) + colourRanges[__lb].g;
	__newCol.b = ( __b * __step1) + colourRanges[__lb].b;

	return __newCol;
}

void OutputColour(Colour _col) {
	DEBUG_PRINT("R: "); DEBUG_PRINTLN(_col.r);
	DEBUG_PRINT("G: "); DEBUG_PRINTLN(_col.g);
	DEBUG_PRINT("B: "); DEBUG_PRINTLN(_col.b);
}

void fadeToNewTemp(float _newTemp)
{
	DEBUG_PRINTLN("Fade to new Temp: " + String(_newTemp, 0));
	Colour __newTempColour = findTempColour(_newTemp);
	if(_isDisplayOn){
		OutputColour(_oldColour);
		OutputColour(__newTempColour);
		fade(_oldColour, __newTempColour);
	}
	_oldColour = __newTempColour;
}
