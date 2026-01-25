#include <SoftwareSerial.h>
#include <FastLED.h>

#define NUM_LEDS_PER_DIGIT 23
#define NUM_LEDS 49
#define RIGHT_DATA_PIN 12
#define LEFT_DATA_PIN 11
#define SERIAL_BAUD_RATE 9600
#define BT_BAUD_RATE 9600
#define NUM_LEDS_SMALL 7


// Setup the bluetooth serial Rx, Tx pins
SoftwareSerial bluetooth(3,2); // RX, TX
char c=' ';
boolean NL = true;
// Variable for bluetooth data
int btData = 0;

// Button pins
const int rightUpBtnPin = 4;
const int rightDownBtnPin = 5;
const int leftUpBtnPin = 7;
const int leftDownBtnPin = 6;
const int resetBtnPin = 9;
const int brightBtnPin = 8;
const int ledPin = 10;
const int gamePin = 13;

// Button variables
int rightUpBtnState = HIGH;
int rightDownBtnState = HIGH;
int leftUpBtnState = HIGH;
int leftDownBtnState = HIGH;
int currentBrightBtnState = HIGH;
int lastBrightBtnState = HIGH;
int currentResetBtnState = HIGH;
int lastResetBtnState = HIGH;

// Variables used to track long press
unsigned long resetPressedTime  = 0;
unsigned long brightPressedTime = 0;
const int shortPressTime = 1000;
const int longPressTime = 3000;
const int colorChangePressTime = 4000;

// Variable to keep general button state
int prestate = 0;
bool isReset = false;
bool isBright = false;
bool isBright2 = false;


// Setup the button codes
const int ERROR = 0;
const int OK = 1;
const int RIGHT_UP = 2;
const int RIGHT_DOWN = 3;
const int LEFT_UP = 4;
const int LEFT_DOWN = 5;
const int RESET = 6;


// Variables to track score
int rightCnt = 0;
int leftCnt = 0;
int gameCnt = 1;

// The array of RGB values assigned to each LED in the strip
CRGB rightLeds[NUM_LEDS];
CRGB leftLeds[NUM_LEDS];
CRGB smallLeds[NUM_LEDS_SMALL];


// Set the initial colors
int hue = 96; // 0 - 255
int smallHue = 0; // 0 - 255
int step = 32;
CRGB rightColor = CHSV(hue, 255, 255);
CRGB leftColor = CHSV(hue, 255, 255);
CRGB smallColor = CHSV(smallHue, 255, 255);


// Set the initial brightness
int ledBrightness = 255; // 0 (off) - 255 (max brightness)

const uint32_t rightDigits[10] = {
  0b00000000011111111100111111111000, // 0
  0b00000000000000011100111000000000, // 1
  0b00000000011111100000111111000111, // 2
  0b00000000000011111100111111000111, // 3
  0b00000000000000011100111000111111, // 4
  0b00000000000011111100000111111111, // 5
  0b00000000011111111100000111111111, // 6
  0b00000000000000011100111111000000, // 7
  0b00000000011111111100111111111111, // 8
  0b00000000000011111100111111111111, // 9
};

const uint32_t leftDigits[10] = {
  //0b00000000000011111111100111111111, // 0
  0b00000000000000000000000000000000, // 0 - unlit
  0b00000000000011100000000000000111, // 1
  0b00000000011111111100000111111000, // 2
  0b00000000011111111100000000111111, // 3
  0b00000000011111100011100000000111, // 4
  0b00000000011100011111100000111111, // 5
  0b00000000011100011111100111111111, // 6
  0b00000000000011111100000000000111, // 7
  0b00000000011111111111100111111111, // 8
  0b00000000011111111111100000111111, // 9
};

const uint8_t smallDigits[10] = {
  0b0001110111, // 0
  0b0001100000, // 1
  0b0001011101, // 2
  0b0001111100, // 3
  0b0001101010, // 4
  0b0000111110, // 5
  0b0000111111, // 6
  0b0000110010, // 7
  0b0001111111, // 8
  0b0001111110, // 9
};

void setup()
{
  // Set pin modes
  pinMode(rightUpBtnPin, INPUT_PULLUP);
  pinMode(rightDownBtnPin, INPUT_PULLUP);
  pinMode(leftUpBtnPin, INPUT_PULLUP);
  pinMode(leftDownBtnPin, INPUT_PULLUP);
  pinMode(brightBtnPin, INPUT_PULLUP);
  pinMode(resetBtnPin, INPUT_PULLUP);

  // Open serial communications
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Starting serial");

  // Set the data rate for the SoftwareSerial port
  bluetooth.begin(BT_BAUD_RATE);
  Serial.print("Bluetooth board started at:");Serial.println(BT_BAUD_RATE);
  Serial.println("Set line endings to 'Both NL & CR'");

  // Initialise the LED strip
  FastLED.addLeds<WS2812B, RIGHT_DATA_PIN, GRB>(rightLeds, NUM_LEDS);
  FastLED.addLeds<WS2812B, LEFT_DATA_PIN, GRB>(leftLeds, NUM_LEDS);
  FastLED.addLeds<WS2812B, gamePin, GRB>(smallLeds, NUM_LEDS_SMALL);
  FastLED.setBrightness(ledBrightness);
}

void loop()
{
  handleBluetooth();
  handleButtonPress();
  setDigit(0, rightCnt, rightColor);
  setDigit(1, leftCnt, leftColor);
  setDigitSmall(gameCnt);
  FastLED.show();
  delay(20);
}

// Set the values in the LED strip corresponding to a particular display/value 
void setDigit(int display, int val, CRGB color){
  for(int i = 0; i < NUM_LEDS; i++){
    // Initialize all LEDs to off
    display == 0 ? rightLeds[i] = CRGB::Black : leftLeds[i] = CRGB::Black;
    
    // Handle right side of digit pair
    if(i < NUM_LEDS_PER_DIGIT && bitRead(rightDigits[val % 10], i))
    {
      display == 0 ? rightLeds[i] = color : leftLeds[i] = color;
    }
    // Handle left side of 
    else if(i > NUM_LEDS_PER_DIGIT + 1 && bitRead(leftDigits[val / 10], i - NUM_LEDS_PER_DIGIT - 3))
    {
      display == 0 ? rightLeds[i] = color : leftLeds[i] = color;
    }
  }
}

void setDigitSmall(int val)
{
  for(int i = 0; i < NUM_LEDS_SMALL; i++){
    // Initialize all LEDs to off
    smallLeds[i] = CRGB::Black;
    
    
    // Handle right side of digit pair
    if(bitRead(smallDigits[val % 10], i))
    {
      smallLeds[i] = smallColor;
      smallLeds[i].fadeLightBy(128);
    }
    
  }
}

// Handles all button presses
void handleButtonPress()
{
  // read the state of the push buttons
  rightUpBtnState = digitalRead(rightUpBtnPin);
  rightDownBtnState = digitalRead(rightDownBtnPin);
  leftUpBtnState = digitalRead(leftUpBtnPin);
  leftDownBtnState = digitalRead(leftDownBtnPin);
  currentBrightBtnState = digitalRead(brightBtnPin);
  currentResetBtnState = digitalRead(resetBtnPin);

  // RIGHT UP
  if (rightUpBtnState == LOW && prestate == 0)
  {
    Serial.println("RIGHT:UP");
    rightCnt = incrementCnt(rightCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // RIGHT DOWN
  else if (rightDownBtnState == LOW && prestate == 0)
  {
    Serial.println("RIGHT:DOWN");
    rightCnt = decrementCnt(rightCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // LEFT UP
  else if (leftUpBtnState == LOW && prestate == 0)
  {
    Serial.println("LEFT:UP");
    leftCnt = incrementCnt(leftCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // LEFT DOWN
  else if (leftDownBtnState == LOW && prestate == 0)
  {
    Serial.println("LEFT:DOWN");
    leftCnt = decrementCnt(leftCnt);
    printScore();
    lightLed();
    prestate = 1;
  }
  // BRIGHTNESS PRESS
  else if (lastBrightBtnState == HIGH && currentBrightBtnState == LOW)
  {
    Serial.println("BRIGHT:PRESSED");
    
    brightPressedTime = millis();
  }
  else if(lastBrightBtnState == LOW && currentBrightBtnState == LOW)
  {
    // Calculate the push duration
    long currentPressDuration = millis() - brightPressedTime;

    if(currentPressDuration > shortPressTime && !isBright)
    {
      Serial.println("COLOR CHANGE");
      //changeColor();
      //flashDigits(3);
      //flashSmallDigit(3);
      isBright = true;
    }
    else if(currentPressDuration > longPressTime && !isBright2)
    {
      Serial.println("SMALL COLOR CHANGE");
      isBright2 = true;
      //flashSmallDigit(3);
      //changeSmallColor();
    }
  }
  else if(lastBrightBtnState == LOW && currentBrightBtnState == HIGH)
  {
    Serial.println("BRIGHT:RELEASED");

    // Calculate the push duration
    long currentPressDuration = millis() - brightPressedTime;

    if(currentPressDuration > longPressTime  && isBright)
    {
      changeSmallColor();
    }
    else if(currentPressDuration > shortPressTime && isBright)
    {
      changeColor();
    }
    if(!isBright)
    {
      adjustBrightness();
    }
    isBright = false;
    isBright2 = false;
  }
  // RESET PRESS
  else if(lastResetBtnState == HIGH && currentResetBtnState == LOW)
  {
    Serial.println("RESET:PRESSED");
    // Capture the press time
    resetPressedTime = millis();
  }
  else if(lastResetBtnState == LOW && currentResetBtnState == LOW)
  {
    // Calculate the push duration
    long currentPressDuration = millis() - resetPressedTime;

    if(currentPressDuration > shortPressTime && !isReset)
    {
      gameCnt = (gameCnt + 1) % 10;
      Serial.print("RESETTING:NEXT:GAME");Serial.println(gameCnt);
      resetCnts();
      printScore();
      isReset = true;
    }
    else if(currentPressDuration > longPressTime && gameCnt != 1)
    {
      Serial.println("RESETTING:GAMES:TO:1");
      gameCnt = 1;
    }
  }
  // RESET RELEASE
  else if(lastResetBtnState == LOW && currentResetBtnState == HIGH)
  {
    Serial.println("RESET:RELEASED");
    isReset = false;
  }
  // NO BUTTON PRESSED
  else if(rightUpBtnState == HIGH && rightDownBtnState == HIGH
    && leftUpBtnState == HIGH && leftDownBtnState == HIGH)
  {
    // Reset the prestate so button presses can be recognized
    prestate = 0;
  }

  // save the the last state
  lastResetBtnState = currentResetBtnState;
  lastBrightBtnState = currentBrightBtnState;
  
}

// Handles incoming bluetooth data
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
      case OK:
        break;
      case RIGHT_UP:
        rightCnt = incrementCnt(rightCnt);
        Serial.println("Sending OK");
        bluetooth.write(OK);
        break;
      case RIGHT_DOWN:
        rightCnt = decrementCnt(rightCnt);
        Serial.println("Sending OK");
        bluetooth.write(OK);
        break;
      case LEFT_UP:
        leftCnt = incrementCnt(leftCnt);
        Serial.println("Sending OK");
        bluetooth.write(OK);
        break;
      case LEFT_DOWN:
        leftCnt = decrementCnt(leftCnt);
        Serial.println("Sending OK");
        bluetooth.write(OK);
        break;
      case RESET:
        resetCnts();
        Serial.println("Sending OK");
        bluetooth.write(OK);
        break;
      default:
        // Recieved garbage, retry
        Serial.println("BT Error, retry");
        bluetooth.write(ERROR);
        break;
    }
    printScore();
  }
}

void handleBluetooth2()
{
// Read from the UART module and send to the Serial Monitor
    if (bluetooth.available())
    {
        c = bluetooth.read();
        Serial.write(c); 
    }
    
    // Read from the Serial Monitor and send to the UART module
    if (Serial.available())
    {
        c = Serial.read();
        
        // do not send line end characters to the HM-10
        if (c!=10 & c!=13 ) {   bluetooth.write(c); }

        // Echo the user input to the main window. 
        // If there is a new line print the ">" character.
        if (NL) { Serial.print("\r\n>");  NL = false; }
        Serial.write(c);
        if (c==10) { NL = true; }
    }
}

// Print the scores to serial
void printScore()
{
  Serial.print("Left: ");
  Serial.print(leftCnt);
  Serial.print(", Right: ");
  Serial.println(rightCnt);
}

// Cycle through brightness settings
void adjustBrightness()
{
  ledBrightness = ledBrightness + 64 % 256;
  FastLED.setBrightness(ledBrightness);
}

void changeColor()
{
  Serial.print("HUE: ");Serial.println(hue);
  hue = (hue + step) % 256;
  rightColor = CHSV(hue, 255, 255);
  leftColor = CHSV(hue, 255, 255);
}

void changeSmallColor()
{
  Serial.print("SMALL HUE: ");Serial.println(hue);
  smallHue = (smallHue + step) % 256; 
  smallColor = CHSV(smallHue, 255, 255);
}

void flashDigits(int numFlashes)
{
  for(int j = 0; j < numFlashes; j++)
  {
    Serial.print("flash");Serial.println(j);
    for(int i = 0; i < NUM_LEDS; i++)
    {
      // Initialize all LEDs to off
      rightLeds[i] = CRGB::Black;
      leftLeds[i] = CRGB::Black;
    }
    delay(2000);
    for(int i = 0; i < NUM_LEDS; i++)
    {
      // Handle right side of digit pair
      if(i < NUM_LEDS_PER_DIGIT && bitRead(rightDigits[rightCnt % 10], i))
      {
        rightLeds[i] = rightColor;
      }
      // Handle left side of 
      else if(i > NUM_LEDS_PER_DIGIT + 1 && bitRead(leftDigits[rightCnt / 10], i - NUM_LEDS_PER_DIGIT - 3))
      {
        rightLeds[i] = rightColor;
      }

      if(i < NUM_LEDS_PER_DIGIT && bitRead(rightDigits[leftCnt % 10], i))
      {
        leftLeds[i] = leftColor;
      }
      // Handle left side of 
      else if(i > NUM_LEDS_PER_DIGIT + 1 && bitRead(leftDigits[leftCnt / 10], i - NUM_LEDS_PER_DIGIT - 3))
      {
        leftLeds[i] = leftColor;
      }
    }
  }
}

void flashSmallDigit(int numFlashes)
{
  for(int j = 0; j < numFlashes; j++)
  {
    for(int i = 0; i < NUM_LEDS_SMALL; i++)
    {
      // Initialize all LEDs to off
      smallLeds[i] = CRGB::Black;
    }
    delay(500);
    for(int i = 0; i < NUM_LEDS_SMALL; i++)
    {
      // Set the specific segements to on
      if(bitRead(smallDigits[gameCnt % 10], i))
      {
        smallLeds[i] = smallColor;
        smallLeds[i].fadeLightBy(128);
      }
    }
  }
}

// Flashes the LED
void lightLed()
{
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);  
}

// Increments a count safely to prevent overflow
int incrementCnt(int cnt)
{
  return (cnt + 1) % 100;
}

// Decrements a count safely to prevent overflow
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

// Resets both score counts
void resetCnts()
{
  rightCnt = 0;
  leftCnt = 0;
}
