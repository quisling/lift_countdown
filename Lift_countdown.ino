// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <LiquidCrystal.h>

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define ARRAY_PIN      6

//Defining the neomatrix and LCD display
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(20, 10, 6,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// Rotary encoder code
int encoderPin1 = 2;
int encoderPin2 = 3;
int encoderSwitchPin = 4; //push button switch

volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastEncoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

//Matrix and color related constants & variables
const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(255, 255, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };
int pass = 0;

//Countdown timer variables
long countdownTimer;//set for default of 20 minutes

unsigned long refreshTimer;
unsigned long liftTime;
unsigned long snapshotTime;
bool snapshotToggle = false;
unsigned long settingTime;

void setup() {
  lcd.begin(16, 2);
  liftTime = millis() + 900000;
  matrix.begin(); // This initializes the NeoPixel library.
  matrix.setTextWrap(false);
  matrix.setBrightness(90);
  matrix.setTextColor(colors[0]);
  matrix.setTextSize(1.2);

  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);
  pinMode(encoderSwitchPin, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
  digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on
  
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);
}

//####### CODE STARTS ######
void loop() {
  unsigned long encoderDeltaTime;
  countdownTimer = (long)((liftTime - millis())/1000);
  long encoderDelta = encoderValue - lastEncoderValue;
  if (!encoderDelta){
    encoderDeltaTime = countdownTimer;
  }else{
    encoderDeltaTime = (unsigned long) ((encoderDelta / 4) * 60) + snapshotTime ;
  }
  
  if(((millis() - settingTime) <= 3000) && !digitalRead(encoderSwitchPin)){
    lcd.clear();
    lcd.print("Teh button don");
    delay(500);
    liftTime = millis() + (encoderDeltaTime * 1000); //rotary gives 4 increases per tick. 
    lastEncoderValue = encoderValue;
    snapshotToggle = false;
  }else if((millis() - settingTime) >3000){
    lastEncoderValue = encoderValue;
    snapshotToggle = false;
  }
  
  paintMatrix(countdownTimer);
  matrix.show();

  if ((millis() - refreshTimer >= 250) || encoderDelta){
    lcd.clear();
    refreshTimer = millis();
  }
  printNewTime(encoderDeltaTime);
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

void paintMatrix(long countdownTimer){
  matrix.fillScreen(1);
  matrix.setCursor(1, 1);
  
  if (countdownTimer <= 60){
    liftEvent();
  }else{
    matrix.print(String(countdownTimer / 600) + " " + String(countdownTimer / 60 % 10));
    for (int i = 0; i <= (int)((countdownTimer % 60) /3); i++){
      matrix.drawPixel(i, 0, colors[pass]);
      matrix.drawPixel(i, 9, colors[pass]);
    }
  }
  pass = (countdownTimer - 60) / 300;
  if (countdownTimer >= 1000){
    pass = 3;
  }
  matrix.setTextColor(colors[pass]);
}

void liftEvent(){
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
}

void printNewTime(int encoderDeltaTime){
  lcd.setCursor(0,0);
  lcd.print("Lift om: " + String(countdownTimer / 60) + ":" + String(countdownTimer % 60));
  lcd.setCursor(0,1);
  lcd.print("Ny tid: " + String(encoderDeltaTime / 60) +  " min");  
  delay(20);
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
  settingTime = millis();
  if(snapshotToggle != true){
    snapshotTime = round(((liftTime - millis())/1000)/60) * 60;
    snapshotToggle = true;
  }
}

