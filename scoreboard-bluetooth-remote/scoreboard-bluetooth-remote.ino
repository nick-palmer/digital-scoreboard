#include <SoftwareSerial.h>

// Setup the bluetooth serial Rx, Tx pins
SoftwareSerial bluetooth(8, 9); // RX, TX

// Button pins
const int homeUpBtnPin = 4;
const int homeDownBtnPin = 3;
const int visUpBtnPin = 6;
const int visDownBtnPin = 5;
const int resetBtnPin = 7;
const int ledPin = 17;

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
  Serial.println("peripheral");

  // Set the data rate for the SoftwareSerial port
  bluetooth.begin(9600);
}

void loop() // run over and over
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
    writeToBluetooth(HOME_UP);
    lightLed();
    prestate = 1;
  }
  // HOME DOWN
  else if (homeDownBtnState == HIGH && prestate == 0)
  {
    Serial.println("HOME:DOWN");
    writeToBluetooth(HOME_DOWN);
    lightLed();
    prestate = 1;
  }
  // VIS UP
  else if (visUpBtnState == HIGH && prestate == 0)
  {
    Serial.println("VIS:UP");
    writeToBluetooth(VIS_UP);
    lightLed();
    prestate = 1;
  }
  // VIS DOWN
  else if (visDownBtnState == HIGH && prestate == 0)
  {
    Serial.println("VIS:DOWN");
    writeToBluetooth(VIS_DOWN);
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
      writeToBluetooth(RESET);
      
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

// Flashes the LED
void lightLed()
{
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);  
}

// Writes the given data to bluetooth
void writeToBluetooth(int datum)
{
  bluetooth.write(datum);
  delay(100);
}
