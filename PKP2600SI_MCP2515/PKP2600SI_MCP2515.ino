/*  Brandon Matthews
 *  PKP-2600-SI controller
 *  Developed for Arduino UNO and MCP2515
 *  Lite Update by Saoud AlNaqbi
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
  colors1[2] = PKP_KEY_CYAN;
  colors1[3] = PKP_KEY_BLUE;
  
  keypad.setKeyColor(PKP_KEY_1, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_2, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_3, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_4, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_5, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_6, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_7, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_8, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_9, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_10, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_11, colors1, blinks1);
  keypad.setKeyColor(PKP_KEY_12, colors1, blinks1);

  keypad.setKeyMode(PKP_KEY_1, BUTTON_MODE_TOGGLE);        // ... master on/off 
  keypad.setKeyMode(PKP_KEY_2, BUTTON_MODE_MOMENTARY);        // ... Starter
  keypad.setKeyMode(PKP_KEY_3, BUTTON_MODE_TOGGLE);        // ... Light
  keypad.setKeyMode(PKP_KEY_4, BUTTON_MODE_CYCLE3);        // ... LED1 LED2
  keypad.setKeyMode(PKP_KEY_5, BUTTON_MODE_TOGGLE);        // ... FAN
  keypad.setKeyMode(PKP_KEY_6, BUTTON_MODE_TOGGLE);        // ... AUX
  keypad.setKeyMode(PKP_KEY_7, BUTTON_MODE_TOGGLE);        // ... Dash LOG
  keypad.setKeyMode(PKP_KEY_8, BUTTON_MODE_TOGGLE);        // .. Radio
  keypad.setKeyMode(PKP_KEY_9, BUTTON_MODE_TOGGLE);        // .. AIR
  keypad.setKeyMode(PKP_KEY_10, BUTTON_MODE_TOGGLE);       // .. Boost
  keypad.setKeyMode(PKP_KEY_11, BUTTON_MODE_MOMENTARY);       // .. Reset 
  keypad.setKeyMode(PKP_KEY_12, BUTTON_MODE_CYCLE4);       // .. Next Page0 Page1 Page2 Page3

  uint8_t defaultStates[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
  keypad.setDefaultButtonStates(defaultStates);

  keypad.begin(CAN_250KBPS, MCP_8MHZ); //These are MCP settings to be passed  
}

//----------------------------------------------------------------------------new

void loop() {
  keypad.process(); //must have this in main loop.

  static uint8_t prevButtonStates[12] = {0}; // initialize the previous button state array

  // check for button state changes and send messages over the CAN bus
  for(int i=0; i<12; i++) {
    if (keypad.buttonState[i] != prevButtonStates[i]) {
      uint8_t buttonState = keypad.buttonState[i];
      prevButtonStates[i] = buttonState;

      // Pack the state value into two bits (bits 0-1)
      uint8_t data = (buttonState & 0x03);

      can_frame frame;
      frame.can_id = 0x300 + i; // use unique CAN ID for each button (1=0x300 ,2=0x301 ,3=0x302 ,4=0x303 ,5=0x304 ,6=0x305 ,7=0x306 ,8=0x307 ,9=0x308 ,10=0x309 ,11=0x30A ,12=0x30B)
      frame.can_dlc = 2; // send two bytes of data
      frame.data[0] = data; // send the button state as the data
      mcp2515.sendMessage(&frame);
    }
  }
}

//----------------------------------------------------------------------------new
