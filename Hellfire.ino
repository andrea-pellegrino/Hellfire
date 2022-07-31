/*******************************************************************************
* Title                 :   Project Hellfire
* Filename              :   Hellfire.ino
* Author                :   Andrea Pellegrino
* Origin Date           :   17/07/2022
* Version               :   0.0
* Compiler              :   avr-gcc
* Target                :   Arduino UNO
* Notes                 :   using Arduino IDE
*******************************************************************************/
/*
 * Description:
 * Bomb SW for SoftAir - Arduino UNO
 *
 *
 *
 * Version log:
 * 0.0: First draft
 */

/******************************************************************************
* Includes
*******************************************************************************/

/******************************************************************************
* Defines
*******************************************************************************/
#define SW_VERSION      0

//I2C Address
#define I2C_ADR_EXP     0x20
#define I2C_ADR_LCD     0x27

//Pins Definition
#define GPIO_LED_B0     A3

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Functions
*******************************************************************************/
/*
 *  Pin Init
 */
 void GPIOInit(void){
     pinMode(GPIO_LED_B0, OUTPUT);
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
    digitalWrite(GPIO_LED_B0, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(GPIO_LED_B0, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);
}

/* EOF */
