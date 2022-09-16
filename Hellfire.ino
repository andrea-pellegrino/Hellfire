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
// Led maps
const byte ledB[5] = {GPIO_LED_B0, GPIO_LED_B1, GPIO_LED_B2, GPIO_LED_B3, GPIO_LED_B4};
const byte ledY[5] = {GPIO_LED_Y0, GPIO_LED_Y1, GPIO_LED_Y2, GPIO_LED_Y3, GPIO_LED_Y4};

// Instances
LiquidCrystal_I2C lcd(I2C_ADR_LCD, 20, 4);   // I2C LCD 20x4 line display
I2CKeyPad keypad(I2C_ADR_KEY);               // I2C Keypad (with PCF8574)

// State variables
mode_e mode = MODE_SELECT;
mode_e selmode = MODE_SELECT;

// Keypad variables
bool keyCurrPress = false;
bool keyLastPress = false;
bool keyRisingEdge = false;
char ch;

// Time variables
char gameTime_str[4];
int gameTime_min;


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
    // LCD Start Message
    lcd.setCursor(0, 0);
    lcd.print("SELECT MODE GAME");
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
 *  Function for key pression
 */
void KeyPress(void) {
    // Check current state of key
    keyCurrPress = keypad.isPressed();
    // Rising edge
    if (keyCurrPress && !keyLastPress) {
        keyRisingEdge = true;
        ch = keypad.getChar();
    }
    else keyRisingEdge = false;
    // Previus state = current state
    keyLastPress = keyCurrPress;
}

/*
 *  Blink LEDs
 */
void BlinkLED(void) {
    static int i;
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
    static bool confirmMode;
    // Select mode from keypad
    if (keyRisingEdge) {
        switch (ch) {
            case 'A':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: A   DOMINATION");
                    selmode = MODE_A_DOMINATION;
                    confirmMode = true;
                }
                break;
            case 'B':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: B    JOINT OP.");
                    selmode = MODE_B_JOINT;
                    confirmMode = true;
                }
                break;
            case 'C':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: C      CLASSIC");
                    selmode = MODE_C_CLASSIC;
                    confirmMode = true;
                }
                break;
            case 'D':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: D       POINTS");
                    selmode = MODE_D_POINTS;
                    confirmMode = true;
                }
                break;
            case '*':
                if (confirmMode) {
                    // Next state
                    mode = MODE_SET_TIME;
                    // LCD 1st line next message
                    lcd.setCursor(0, 0);
                    lcd.print("SET GAME TIME (min):");
                    // Reset 2nd line
                    lcd.setCursor(0, 1);
                    lcd.print("           (max 999)");
                    // Reset 3rd line
                    lcd.setCursor(0, 3);
                    lcd.print("                    ");
                    // Blink cursor
                    lcd.setCursor(0, 1);
                    lcd.blink();
                    // Reset lights
                    for (uint8_t i = 0; i < 5; i++) {
                        digitalWrite(ledB[i], LOW);
                        digitalWrite(ledY[i], LOW);
                    }
                }
                break;
            case '#':
                if (confirmMode) {
                    selmode = MODE_SELECT;
                    // Reset 2nd & 4th line
                    lcd.setCursor(0, 1);
                    lcd.print("                    ");
                    lcd.setCursor(0, 3);
                    lcd.print("                    ");
                    confirmMode = false;
                }
                break;
            default:
                break;
        }
    }
}

/*
 *  Set game time
 */
void SetTime(void) {
    static int x;
    static bool confirmTime;
    // Select game time in minutes from keypad
    if (keyRisingEdge) {
        switch (ch) {
            case '0'...'9':
                if ((!confirmTime) && (x >= 0) && (x <= 2)) {
                    lcd.setCursor(x, 1);
                    lcd.print(ch);
                    gameTime_str[x] = ch;
                    x++;
                }
                break;
            case '*':
                if ((!confirmTime) && (x > 0)) {
                    gameTime_min = atoi(gameTime_str);
                    lcd.noBlink();
                    lcd.setCursor(0, 2);
                    lcd.print("CONFIRM?            ");
                    confirmTime = true;
                }
                else if (confirmTime) {
                    lcd.clear();
                }
                break;
            case '#':
                if ((!confirmTime) && (x >= 1) && (x <= 3)) {
                    x--;
                    lcd.setCursor(x, 1);
                    lcd.print(" ");
                    lcd.setCursor(x, 1);
                    gameTime_str[x] = '\0';
                }
                else if ((!confirmTime) && (x > 3)) {
                    lcd.blink();
                    confirmTime = false;
                }
                else if (confirmTime) {
                    // Reset 3rd line
                    lcd.setCursor(0, 2);
                    lcd.print("                    ");
                    // Blink cursor
                    lcd.setCursor(x, 1);
                    lcd.blink();
                    confirmTime = false;
                }
                break;
            default:
                break;
        }
    }
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
            KeyPress();
            BlinkLED();
            SelectMode();
            break;
        case MODE_SET_TIME:
            KeyPress();
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
