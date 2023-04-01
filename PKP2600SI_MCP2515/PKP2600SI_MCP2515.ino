/*  Brandon Matthews
 *  PKP-2600-SI controller
 *  Developed for Arduino UNO and MCP2515
 *  
 */

//This library depends on and requires TimerOne library, SPI library, and autowp mcp2515 library v1.03
#include "PKP2600SI_CANOPEN.h"

#define CS_PIN 10
#define INTERRUPT_PIN 3
#define KEYPAD_BASE_ID 0x15
#define ENABLE_PASSCODE false

MCP2515 mcp2515(CS_PIN);
CANKeypad keypad(mcp2515, INTERRUPT_PIN, KEYPAD_BASE_ID, ENABLE_PASSCODE); 


void setup() {
  Serial.begin(115200);
  keypad.setSerial(&Serial); //Required for the keypad library to print things out to serial

  uint8_t keypadPasscode[4] = {1,2,3,4};
  keypad.setKeypadPassword(keypadPasscode);
  keypad.setKeyBrightness(70);
  keypad.setBacklightBrightness(PKP_BACKLIGHT_AMBER, 10);

  //Set Key color and blink states
  uint8_t colors1[4] = {PKP_KEY_BLANK,PKP_KEY_RED,PKP_KEY_BLANK,PKP_KEY_BLANK}; //array for the 4 possible button states' respective colors
  uint8_t blinks1[4] = {PKP_KEY_BLANK,PKP_KEY_BLANK,PKP_KEY_BLANK,PKP_KEY_BLANK};
  colors1[1] = PKP_KEY_GREEN;
  keypad.setKeyColor(PKP_KEY_1, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_2, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_3, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_4, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_5, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_6, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_7, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_8, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_9, colors1, blinks1);
  colors1[1] = PKP_KEY_GREEN;
  colors1[2] = PKP_KEY_CYAN;
  keypad.setKeyColor(PKP_KEY_10, colors1, blinks1);
  colors1[1] = PKP_KEY_GREEN;  
  keypad.setKeyColor(PKP_KEY_11, colors1, blinks1);
  colors1[1] = PKP_KEY_GREEN;
  colors1[2] = PKP_KEY_CYAN;
  colors1[3] = PKP_KEY_BLUE;   
  keypad.setKeyColor(PKP_KEY_12, colors1, blinks1);

  keypad.setKeyMode(PKP_KEY_1, BUTTON_MODE_MOMENTARY);        // ... master on/off 
  keypad.setKeyMode(PKP_KEY_2, BUTTON_MODE_MOMENTARY);     // ... Starter
  keypad.setKeyMode(PKP_KEY_3, BUTTON_MODE_MOMENTARY);        // ... Light
  keypad.setKeyMode(PKP_KEY_4, BUTTON_MODE_MOMENTARY);        // ... LED
  keypad.setKeyMode(PKP_KEY_5, BUTTON_MODE_MOMENTARY);        // ... FAN
  keypad.setKeyMode(PKP_KEY_6, BUTTON_MODE_MOMENTARY);        // ... AUX
  keypad.setKeyMode(PKP_KEY_7, BUTTON_MODE_MOMENTARY);        // ... Dash LOG
  keypad.setKeyMode(PKP_KEY_8, BUTTON_MODE_MOMENTARY);        // .. Radio
  keypad.setKeyMode(PKP_KEY_9, BUTTON_MODE_MOMENTARY);        // .. AIR
  keypad.setKeyMode(PKP_KEY_10, BUTTON_MODE_MOMENTARY);       // .. Boost
  keypad.setKeyMode(PKP_KEY_11, BUTTON_MODE_MOMENTARY);    // .. Reset 
  keypad.setKeyMode(PKP_KEY_12, BUTTON_MODE_MOMENTARY);       // .. Next

  uint8_t defaultStates[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
  keypad.setDefaultButtonStates(defaultStates);

  keypad.begin(CAN_250KBPS, MCP_8MHZ); //These are MCP settings to be passed  
}

//----------------------------------------------------------------------------

void loop() {
  keypad.process(); //must have this in main loop.

  static bool prevButtonStates[12] = {0}; // initialize the previous button state array
  static bool buttonToggled[12] = {0}; // initialize the button toggled array
  static bool counterStates[12] = {0}; // initialize the counter state array

  // check for button state changes and send messages over the CAN bus
  for(int i=0; i<12; i++) {
    if (keypad.buttonState[i] != prevButtonStates[i]) {
      bool buttonState = keypad.buttonState[i];
      prevButtonStates[i] = buttonState;

      // check if the button state has changed
      if (buttonState != buttonToggled[i]) {
        buttonToggled[i] = buttonState;

        // if the button state has gone from 0 to 1, toggle the counter state
        if (buttonState == true) {
          counterStates[i] = !counterStates[i];
        }

        can_frame frame;
        frame.can_id = 0x300 + i; // use unique CAN ID for each button
        frame.can_dlc = 1;
        frame.data[0] = counterStates[i] ? 1 : 0; // send 1 if the counter state is true, else send 0
        mcp2515.sendMessage(&frame);
      }
    }
  }
}
