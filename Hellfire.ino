/*******************************************************************************
* Title                 :   Project Hellfire
* Filename              :   Hellfire.ino
* Author                :   Andrea Pellegrino
* Origin Date           :   17/07/2022
* Version               :   0.0
* Compiler              :   avr-gcc
* Target                :   Arduino UNO (avr)
* Notes                 :   using arduino-cli
*******************************************************************************/
/*
 * Description:
 * Bomb SW for SoftAir - Arduino UNO
 *
 *
 *
 * Version log:
 * 0.0: First draft - 17/07/2022
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "I2CKeyPad.h"              // I2C Keypad Library
#include "LiquidCrystal_I2C.h"      // I2C LCD Screen Library
#include "Wire.h"                   // I2C Library

/******************************************************************************
* Defines
*******************************************************************************/
// Software Version
#define SW_VERSION      0

// I2C Address
#define I2C_ADR_KEY     0x20        // I2C Keypad Address (I2C Expansion PCF8574)
#define I2C_ADR_LCD     0x27        // I2C LCD Address

// Pins Definition
#define GPIO_LED_B0     A3
#define GPIO_LED_B1     A2
#define GPIO_LED_B2     A1
#define GPIO_LED_B3     1
#define GPIO_LED_B4     0
#define GPIO_LED_Y0     A0
#define GPIO_LED_Y1     13
#define GPIO_LED_Y2     12
#define GPIO_LED_Y3     11
#define GPIO_LED_Y4     10
#define GPIO_BZR        9
#define GPIO_SW_GRN     8
#define GPIO_SW_RED     7

/******************************************************************************
* Enumerations
*******************************************************************************/
// Game mode (main loop FSM)
enum mode_e {
    MODE_SELECT,
    MODE_SET_TIME,
    MODE_A_DOMINATION,
    MODE_B_JOINT,
    MODE_C_CLASSIC,
    MODE_D_POINTS
};

/******************************************************************************
* Variables
*******************************************************************************/
// Keypad map
const char keymap[19] = "123A456B789C*0#DNF";  // N = NoKey, F = Fail
// Led map
const byte ledB[5] = {GPIO_LED_B0, GPIO_LED_B1, GPIO_LED_B2, GPIO_LED_B3, GPIO_LED_B4};
const byte ledY[5] = {GPIO_LED_Y0, GPIO_LED_Y1, GPIO_LED_Y2, GPIO_LED_Y3, GPIO_LED_Y4};

// Instances
LiquidCrystal_I2C lcd(I2C_ADR_LCD, 20, 4);   // I2C LCD 20x4 line display
I2CKeyPad keypad(I2C_ADR_KEY);               // I2C Keypad (with PCF8574)

// State variable
mode_e mode = MODE_SELECT;
mode_e selmode = MODE_SELECT;

/******************************************************************************
* Init Functions
*******************************************************************************/
/*
 *  GPIO Pin Initialization
 */
void GPIO_Init(void) {
    pinMode(GPIO_LED_B0, OUTPUT);
    pinMode(GPIO_LED_B1, OUTPUT);
    pinMode(GPIO_LED_B2, OUTPUT);
    pinMode(GPIO_LED_B3, OUTPUT);
    pinMode(GPIO_LED_B4, OUTPUT);
    pinMode(GPIO_LED_Y0, OUTPUT);
    pinMode(GPIO_LED_Y1, OUTPUT);
    pinMode(GPIO_LED_Y2, OUTPUT);
    pinMode(GPIO_LED_Y3, OUTPUT);
    pinMode(GPIO_LED_Y4, OUTPUT);
    pinMode(GPIO_BZR, OUTPUT);
    pinMode(GPIO_SW_GRN, OUTPUT);
    pinMode(GPIO_SW_RED, OUTPUT);
}

/*
 *  I2C Wire Initialization
 */
void I2C_Init(void) {
    Wire.begin();
    Wire.setClock(100000);
}

/*
 *  LCD I2C Initialization
 */
void LCD_Init(void) {
    lcd.init();
    lcd.backlight();
}

/*
 *  Keypad I2C Initialization
 */
void Keypad_Init(void) {
    keypad.begin();
    keypad.loadKeyMap(keymap);
}

/******************************************************************************
* Generic Functions
*******************************************************************************/
/*
 *  Blink LEDs
 */
void BlinkLED(void) {
    static uint8_t i;
    if (i < 5)                       digitalWrite(ledB[i], HIGH);
    else if ((i >= 5) && (i < 10))   digitalWrite(ledY[i%5], HIGH);
    else if ((i >= 10) && (i < 15))  digitalWrite(ledB[i%5], LOW);
    else if ((i >= 15) && (i < 20))  digitalWrite(ledY[i%5], LOW);
    //Check
    i++;
    if (i >= 20) i = 0;
    //Delay
    delay(50);
}

/*
 *  Start select mode
 */
void SelectMode(void) {
    char ch;
    // LCD Message
    lcd.setCursor(0, 0);
    lcd.print("SELECT MODE GAME");
    // Select mode from keypad
    if (keypad.isPressed())  ch = keypad.getChar();
    switch (ch) {
        case 'A':
            lcd.setCursor(0, 1);
            lcd.print("CONFIRM SELECTION?");
            lcd.setCursor(0, 3);
            lcd.print("MODE: A   DOMINATION");
            selmode = MODE_A_DOMINATION;
            break;
        case 'B':
            lcd.setCursor(0, 1);
            lcd.print("CONFIRM SELECTION?");
            lcd.setCursor(0, 3);
            lcd.print("MODE: B    JOINT OP.");
            selmode = MODE_B_JOINT;
            break;
        case 'C':
            lcd.setCursor(0, 1);
            lcd.print("CONFIRM SELECTION?");
            lcd.setCursor(0, 3);
            lcd.print("MODE: C      CLASSIC");
            selmode = MODE_C_CLASSIC;
            break;
        case 'D':
            lcd.setCursor(0, 1);
            lcd.print("CONFIRM SELECTION?");
            lcd.setCursor(0, 3);
            lcd.print("MODE: D       POINTS");
            selmode = MODE_D_POINTS;
            break;
        case '*':
            // Next state
            mode = MODE_SET_TIME;
            // Reset 2nd line
            lcd.setCursor(0, 1);
            lcd.print("                    ");
            // Reset lights
            for (uint8_t i = 0; i < 5; i++) {
                digitalWrite(ledB[i], LOW);
                digitalWrite(ledY[i], LOW);
            }
            break;
        case '#':
            selmode = MODE_SELECT;
            // Reset 2nd & 4th line
            lcd.setCursor(0, 3);
            lcd.print("                    ");
            lcd.setCursor(0, 1);
            lcd.print("                    ");
            break;
        default:
            break;
    }
}

/*
 *  Set game time
 */
void SetTime(void) {
    char ch;
    // LCD Message
    lcd.setCursor(0, 0);
    lcd.print("SET GAME TIME (min):");
}

 /******************************************************************************
* Main Arduino Functions (Setup + Loop)
*******************************************************************************/
/*
 *  Setup main function
 */
void setup() {
    // Init
    GPIO_Init();
    I2C_Init();
    LCD_Init();
    Keypad_Init();
}

/*
 *  Loop main function
 */
void loop() {
    // Finite State Machine
    switch (mode) {
        case MODE_SELECT:
            BlinkLED();
            SelectMode();
            break;
        case MODE_SET_TIME:
            SetTime();
            break;
        case MODE_A_DOMINATION:
            break;
        case MODE_B_JOINT:
            break;
        case MODE_C_CLASSIC:
            break;
        case MODE_D_POINTS:
            break;
        default:
            break;
    }
}

/* EOF */
