#include <SoftwareSerial.h>
#include <FastLED.h>

#define NUM_LEDS_PER_DIGIT 23
#define NUM_LEDS 49
#define DATA_PIN 11

// Setup the bluetooth serial Rx, Tx pins
SoftwareSerial bluetooth(8, 9); // RX, TX

// Button pins
const int homeUpBtnPin = 4;
const int homeDownBtnPin = 3;
const int visUpBtnPin = 6;
const int visDownBtnPin = 5;
const int resetBtnPin = 7;
const int ledPin = 13;

// Button variables
int homeUpBtnState = LOW;
int homeDownBtnState = LOW;
int visUpBtnState = LOW;
int visDownBtnState = LOW;
int currentResetBtnState = LOW;
int lastResetBtnState = LOW;

// Variables used to track long press
unsigned long resetPressedTime  = 0;
unsigned long resetReleasedTime = 0;
const int shortPressTime = 2000;

// Variable to keep general button state
int prestate = 0;

// Setup the button codes
const int HOME_UP = 1;
const int HOME_DOWN = 2;
const int VIS_UP = 3;
const int VIS_DOWN = 4;
const int RESET = 5;

// Variables to track score
int homeCnt = 0;
int visCnt = 0;
int gameCnt = 0;

// Variable for bluetooth data
int btData = 0;

// The array of RGB values assigned to each LED in the strip
CRGB leds[NUM_LEDS];
int ledBrightness = 32; // 0 (off) - 255 (max brightness)
const uint32_t rightDigits[10] = {
  0b00000000000011111111100111111111, // 0
  0b00000000000000000011100111000000, // 1
  0b00000000000011111100000111111111, // 2
  0b00000000011100011111100111111000, // 3
  0b00000000011100000011100111000111, // 4
  0b00000000011100011111100000111111, // 5
  0b00000000011111111111100000111111, // 6
  0b00000000000000000011100111111111, // 7
  0b00000000011111111111100111111111, // 8
  0b00000000011100011111100111111111, // 9
};

//const uint32_t leftDigits[10] = {
//  0b00000000000011111111100111111111, // 0
//  0b00000000000011100000000000000111, // 1
//  0b00000000011100011111100000111111, // 2
//  0b00000000011111111100000000111111, // 3
//  0b00000000011111100000000111000111, // 4
//  0b00000000011111111100000111111000, // 5
//  0b00000000011111111111100111111000, // 6
//  0b00000000000011100000000000111111, // 7
//  0b00000000011111111111100111111111, // 8
//  0b00000000011111111100000111111111, // 9
//};

const uint32_t leftDigits[10] = {
  0b00000000011111111100111111111000, // 0
  0b00000000011100000000000000111000, // 1
  0b00000000011111100000111111000111, // 2
  0b00000000011111100000000111111111, // 3
  0b00000000011100011100000000111111, // 4
  0b00000000000011111100000111111111, // 5
  0b00000000000011111100111111111111, // 6
  0b00000000011111100000000000111000, // 7
  0b00000000011111111100111111111111, // 8
  0b00000000011111111100000111111111, // 9
};

// Set the values in the LED strip corresponding to a particular display/value 
void setDigit(int display, int val, CRGB color){
  for(int i = 0; i < NUM_LEDS_PER_DIGIT; i++){
    //Serial.print(bitRead(rightDigits[val], i));
    leds[display * NUM_LEDS_PER_DIGIT + i] = CRGB::Black;
    if(bitRead(leftDigits[val], i))
    {
      leds[display * NUM_LEDS_PER_DIGIT + i] = CRGB::Blue;
    }
  }
   //Serial.println("DONE");
}

void setup()
{
  // Set pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(homeUpBtnPin, INPUT);
  pinMode(homeDownBtnPin, INPUT);
  pinMode(visUpBtnPin, INPUT);
  pinMode(visDownBtnPin, INPUT);
  pinMode(resetBtnPin, INPUT_PULLUP);

  // Open serial communications
  Serial.begin(9600);
  Serial.println("Starting serial");

  // Set the data rate for the SoftwareSerial port
  bluetooth.begin(9600);

  // Initialise the LED strip
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(ledBrightness);
}

void loop()
{
  handleBluetooth();
  handleButtonPress();
  setDigit(0, homeCnt, CRGB(255,255,0));
  FastLED.show();
  delay(20);
}

void handleButtonPress()
{
  // read the state of the push buttons
  homeUpBtnState = digitalRead(homeUpBtnPin);
  homeDownBtnState = digitalRead(homeDownBtnPin);
  visUpBtnState = digitalRead(visUpBtnPin);
  visDownBtnState = digitalRead(visDownBtnPin);
  currentResetBtnState = digitalRead(resetBtnPin);

  // HOME UP
  if (homeUpBtnState == HIGH && prestate == 0)
  {
    Serial.println("HOME:UP");
    homeCnt = incrementCnt(homeCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // HOME DOWN
  else if (homeDownBtnState == HIGH && prestate == 0)
  {
    Serial.println("HOME:DOWN");
    homeCnt = decrementCnt(homeCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // VIS UP
  else if (visUpBtnState == HIGH && prestate == 0)
  {
    Serial.println("VIS:UP");
    visCnt = incrementCnt(visCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // VIS DOWN
  else if (visDownBtnState == HIGH && prestate == 0)
  {
    Serial.println("VIS:DOWN");
    visCnt = decrementCnt(visCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // RESET PRESS
  else if(lastResetBtnState == LOW && currentResetBtnState == HIGH)
  {
    Serial.println("RESET:PRESSED");
    // Capture the press time
    resetPressedTime = millis();
  }
  // RESET RELEASE
  else if(lastResetBtnState == HIGH && currentResetBtnState == LOW)
  {
    Serial.println("RESET:RELEASED");
    // Capture the release time
    resetReleasedTime = millis();

    // Calculate the push duration
    long pressDuration = resetReleasedTime - resetPressedTime;

    // If the reset button was long pressed
    if( pressDuration > shortPressTime )
    {
      Serial.println("RESETTING");
      resetCnts();
      printScore();
      
      // Flash LEDs
      lightLed();
      delay(500);
      lightLed();
      delay(500);
      lightLed();
    }
  }
  // NO BUTTON PRESSED
  else if(homeUpBtnState == LOW && homeDownBtnState == LOW
    && visUpBtnState == LOW && visDownBtnState == LOW)
  {
    // Reset the prestate so button presses can be recognized
    prestate = 0;
  }

  // save the the last state
  lastResetBtnState = currentResetBtnState;
}

void handleBluetooth()
{
  // Handle incoming data
  if(bluetooth.available() > 0)
  {
    btData = bluetooth.read();
    Serial.print("btData:");
    Serial.println(btData);

    switch(btData)
    {
      case HOME_UP:
        homeCnt = incrementCnt(homeCnt);
        break;
      case HOME_DOWN:
        homeCnt = decrementCnt(homeCnt);
        break;
      case VIS_UP:
        visCnt = incrementCnt(visCnt);
        break;
      case VIS_DOWN:
        visCnt = decrementCnt(visCnt);
        break;
      case RESET:
        resetCnts();
        break;
      default:
        break;
    }
    printScore();
  }
}

void printScore()
{
  Serial.print("HOME: ");
  Serial.print(homeCnt);
  Serial.print(", VISITOR: ");
  Serial.println(visCnt);
}

// Flashes the LED
void lightLed()
{
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);  
}

int incrementCnt(int cnt)
{
  return (cnt + 1) % 100;
}

int decrementCnt(int cnt)
{
  if(cnt != 0)
  {
    return (cnt - 1) % 100;
  }
  else
  {
    return 99;
  }
}

void resetCnts()
{
  homeCnt = 0;
  visCnt = 0;
}
