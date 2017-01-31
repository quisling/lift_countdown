// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define ARRAY_PIN      6
#define ANALOG_PIN     2

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(20, 10, 6,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);
  
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ARRAY_PIN, NEO_GRB + NEO_KHZ800);

int delayval = 10; // delay for half a second
int color = 0;
int last_level = 0;
int colorval = 0;
int countdownTimer = 800;
// Rotary encoder code
int encoderPin1 = 2;
int encoderPin2 = 3;

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;


const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };


int x    = matrix.width();
int pass = 0;

void setup() {
  matrix.begin(); // This initializes the NeoPixel library.
  matrix.setTextWrap(false);
  matrix.setBrightness(90);
  matrix.setTextColor(colors[0]);
  matrix.setTextSize(1.2);

  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);
}

void loop() {
  matrix.fillScreen(1);
  matrix.setCursor(1, 1);
  int high_minutes = countdownTimer / 600;
  int low_minutes = countdownTimer / 60;
  if (countdownTimer <= 60){
    matrix.setCursor(0,1);
    matrix.print("L");
    matrix.setCursor(5,1);
    matrix.print("I");
    matrix.setCursor(10,1);
    matrix.print("F");
    matrix.setCursor(15,1);
    matrix.print("T");
    matrix.drawPixel(14, 1, matrix.Color(0,0,0));
    matrix.drawPixel(13, 4, matrix.Color(0,0,0));
  }else{
    String lol = String(low_minutes);
    matrix.print(String(high_minutes) + " " + String(low_minutes % 10));
    for (int i = 0; i <= ((countdownTimer % 60) /3); i++){
      matrix.drawPixel(i, 0, colors[pass]);
      matrix.drawPixel(i, 9, colors[pass]);
    }
  }
  pass = (countdownTimer - 60) / 300;
  matrix.setTextColor(colors[pass]);
  matrix.show();
  if (countdownTimer > 0){
    countdownTimer--;
  }
  delay(20);
  
  if (encoderValue >= 255) {
    encoderValue = 0;
  } 
  for (int i = 0; i < matrix.numPixels(); i++) {
    // pixels.setPixelColor(i, Wheel(encoderValue));
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
}
