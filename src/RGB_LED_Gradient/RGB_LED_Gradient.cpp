// Do not remove the include below
#include "RGB_LED_Gradient.h"

#include <OneWire.h>
#include <DallasTemperature.h>

/*
  This sketch reads the temperatures off DS18B20 OneWire sensors,
  outputs these values via the serial port, and visualizes them
  via a set of R(G)B-LEDs using a relative red-blue gradient.

  The circuit:
  * Alternating red and blue LEDs attached from pins 44, 45, and 2 through 13 to ground (i. e. odd = red).
  * OneWire bus attached to pin 53.

  Adapted from a public domain example by Tom Igoe in 2009.

 */
/*
1, 2857628204000067
2, 28610382040000E4
3, 2852F681040000AC
4, 281694820400001F
5, 289D2F8204000032
6, 28534882040000FE
7, 28F59E82040000D2
*/

const uint8_t oneWirePin = 53;

OneWire oneWire(oneWirePin);
DallasTemperature sensors(&oneWire);

int numSensors = 0;

const uint8_t pins[] = { 44, 45, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const uint8_t numPins = sizeof(pins) / sizeof(pins[0]);
const uint8_t numLeds = numPins / 2;

uint8_t rampR[numLeds];
uint8_t rampB[numLeds];

float tMin = 0.0;
float tMax = 0.0;

// maximum PWM duty cycle, i. e. max LED brightness
const uint8_t maxBrightness = 16;

// set LED n off / on, color being controlled by colorIndex
void setLED(uint8_t n, bool on = false, uint8_t colorIndex = 0) {
	const uint8_t pinR = pins[2 * n];
	const uint8_t pinB = pins[2 * n + 1];
	analogWrite(pinR, on ? rampR[colorIndex] : 0);
	analogWrite(pinB, on ? rampB[colorIndex] : 0);
}

void log_address(uint8_t index)
{
	uint8_t address[8];
	sensors.getAddress(address, index);
	for(int i = 0; i < 8; i++)
	{
		if(address[i] < 16)
		{
			Serial.print(0);
		}
		Serial.print(address[i], HEX);
	}
}

uint8_t temp2index(const float& temp) {
	return (tMin == tMax) ? (numLeds / 2) : numLeds * (temp - tMin) / (tMax - tMin);
}

void setup() {
	Serial.begin(115200);
	Serial.println("#TCC Data Logger Boot");

	// Start up the 1wire Dallas library
  sensors.begin();

  // Find 1wire devices on the 1wire bus
  numSensors = sensors.getDeviceCount();

  Serial.print("#Sensors: ");
  Serial.println(numSensors);

  // set LED pins as outputs:
  for (uint8_t i = 0; i < numPins; ++i) {
    pinMode(pins[i], OUTPUT);
  }

  // set up color ramps
  float f = 1.0 * maxBrightness / numLeds;
	for (uint8_t i = 0; i < numLeds; ++i) {
		// red: 0 to maxBrightness, numLeds steps
		rampR[i] = (uint8_t)(i * f);
		// blue: maxBrightness down to 0, numLeds steps
		rampB[i] = (uint8_t)((numLeds - i - 1) * f);
	}

	// Show LED test pattern upon startup
//  while(true){
		for (uint8_t i = 0; i < numLeds; i++) {
			setLED(i, true, numLeds);
			delay(100);
			setLED(i, true, 0);
			delay(100);
			setLED(i, false);
		}
		delay(500);
//  }
	for (uint8_t i = 0; i < numLeds; i++) {
			setLED(i, true, numLeds);
	}
	delay(2000);
	for (uint8_t i = 0; i < numLeds; i++) {
			setLED(i, true, i);
	}
	delay(2000);
	for (uint8_t i = 0; i < numLeds; i++) {
			setLED(i, true, numLeds - i);
	}
	delay(2000);
}

void loop() {
	static float temps[numLeds];

	sensors.begin();
	numSensors = min(sensors.getDeviceCount(), numLeds);

	sensors.requestTemperatures();
	for (int i = 0 ; i < numSensors ; i++ ) {
		float t = temps[i] = sensors.getTempCByIndex(i);
		if (i == 0) {
			tMin = tMax = t;
		}
		tMin = (tMin < t) ? tMin : t;
		tMax = (tMax > t) ? tMax : t;
		Serial.print("Temp #");
		Serial.print(i);
		Serial.print(" / ");
		log_address(i);
		Serial.print(": ");
		Serial.println(t);
	}
	for (uint8_t i = 0; i < numLeds; i++) {
			setLED(i, i < numSensors, temp2index(temps[i]));
	}

	Serial.println();
//	delay(500);
}
