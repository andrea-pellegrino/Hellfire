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
    MODE_START,
    MODE_A_DOMINATION,
    MODE_B_JOINT,
    MODE_C_CLASSIC,
    MODE_D_POINTS
};

/******************************************************************************
* Variables
*******************************************************************************/
// Instances
LiquidCrystal_I2C lcd(I2C_ADR_LCD, 20, 4);   // I2C LCD 20x4 line display
I2CKeyPad keypad(I2C_ADR_KEY);               // I2C Keypad (with PCF8574)

// State variable
mode_e mode = MODE_START;

// Keypad map
char keymap[19] = "123A456B789C*0#DNF";  // N = NoKey, F = Fail

/******************************************************************************
* Init Functions
*******************************************************************************/
/*
 *  GPIO Pin Initialization
 */
void GPIO_Init(void){
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
* FSM Functions
*******************************************************************************/
/*
 *  Start Loop Function
 */
void Start_Loop() {

}

/*
 *  Mode A Loop Function (DOMINATION)
 */
void Mode_A_Loop() {

}

/*
 *  Mode B Loop Function (JOINT)
 */
void Mode_B_Loop() {

}

/*
 *  Mode C Loop Function (CLASSIC)
 */
void Mode_C_Loop() {

}

/*
 *  Mode D Loop Function (POINTS)
 */
void Mode_D_Loop() {

}

 /******************************************************************************
* Main Arduino Functions (Setup + Loop)
*******************************************************************************/
/*
 *  Setup main function
 */
void setup() {
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
        case MODE_START:
            Start_Loop();
            break;
        case MODE_A_DOMINATION:
            Mode_A_Loop();
            break;
        case MODE_B_JOINT:
            Mode_B_Loop();
            break;
        case MODE_C_CLASSIC:
            Mode_C_Loop();
            break;
        case MODE_D_POINTS:
            Mode_D_Loop();
            break;
        default:
            break;
    }
}

/* EOF */
