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

/******************************************************************************
* Defines
*******************************************************************************/
// Software Version
#define SW_VERSION      0

// I2C Address
#define I2C_ADR_EXP     0x20
#define I2C_ADR_LCD     0x27

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
mode_e mode = MODE_START;

/******************************************************************************
* Functions
*******************************************************************************/
/*
 *  GPIO Pin Initialization
 */
 void GPIOInit(void){
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

/******************************************************************************
* Main Arduino Functions (Setup + Loop)
*******************************************************************************/
/*
 *  Setup main function
 */
void setup() {
    GPIOInit();
}

/*
 *  Loop main function
 */
void loop() {
    // Finite State Machine
    switch (mode) {
        case MODE_START:
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
