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
 * Version log:
 * 0.0: First draft
 */

/******************************************************************************
* Includes
*******************************************************************************/
// Public libraries
#include "I2CKeyPad.h"              // I2C Keypad Library
#include "LiquidCrystal_I2C.h"      // I2C LCD Screen Library
#include "Wire.h"                   // I2C Library

// Private libraries
#include "LedBar.h"                 // Led Bar Library
#include "TimeCount.h"              // Time Counter Library

/******************************************************************************
* Defines
*******************************************************************************/
// Software Version
#define SW_VERSION      0
#define SW_REVISION     0

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
    MODE_SET_CODE,
    MODE_START_GAME,
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
const byte BlueMap[5] = {GPIO_LED_B0, GPIO_LED_B1, GPIO_LED_B2, GPIO_LED_B3, GPIO_LED_B4};
const byte YelMap[5] = {GPIO_LED_Y0, GPIO_LED_Y1, GPIO_LED_Y2, GPIO_LED_Y3, GPIO_LED_Y4};

// Instances
LiquidCrystal_I2C lcd(I2C_ADR_LCD, 20, 4);   // I2C LCD 20x4 line display
I2CKeyPad keypad(I2C_ADR_KEY);               // I2C Keypad (with PCF8574)
LedBar ledB(BlueMap, 5);                     // Blue LED bar
LedBar ledY(YelMap, 5);                      // Yellow LED bar
TimeCount GameTimer;                         // Timer of Game

// State variables
mode_e mode = MODE_SELECT;
mode_e selMode = MODE_SELECT;

// Keypad variables
bool keyCurrPress = false;
bool keyLastPress = false;
char ch;

// Time variables
char gameTime_str[4];
unsigned int gameTime_min;

/******************************************************************************
* Generic Functions
*******************************************************************************/
/*
 *  Function for key pression rising edge
 */
bool KeyRising(void) {
     bool keyRisingEdge;        // Return value = 1 when key is pressed

     /* Keypad pression state */
     keyCurrPress = keypad.isPressed();

     /* Keypad rising edge */
     if (keyCurrPress && !keyLastPress) {
         keyRisingEdge = true;
         ch = keypad.getChar();
     }
     else keyRisingEdge = false;

     /* Update pression state */
     keyLastPress = keyCurrPress;

     /* Return value */
     return keyRisingEdge;
 }

/******************************************************************************
* Init Functions
*******************************************************************************/
/*
 *  I2C Wire Initialization
 */
 void I2C_Init(void) {

     /* I2c begin */
     Wire.begin();

     /* I2c speed at 100k */
     Wire.setClock(100000);
 }

/*
 *  LCD I2C Initialization
 */
void LCD_Init(void) {
    char version_str[8];    // Current version string: (es: v1.2)

    /* LCD display init */
    lcd.init();

    /* Display backlight on */
    lcd.backlight();

    /* Display Start Message */
    lcd.setCursor(0, 0);
    lcd.print("SELECT MODE GAME");
    lcd.setCursor(0, 3);
    snprintf(version_str, 8, "v%d.%d", SW_VERSION, SW_REVISION);
    lcd.print(version_str);
}

/*
 *  Keypad I2C Initialization
 */
void Keypad_Init(void) {

    /* Keypad begin */
    keypad.begin();

    /* Keypad load key map */
    keypad.loadKeyMap(keymap);
}

/******************************************************************************
* Loop Functions
*******************************************************************************/
/*
 *  Mode Select loop function
 */
void ModeSelect_Loop(void) {
    static bool confirmMode;        // 0-> select mode 1-> confirm selected mode

    /* Blink led bars */
    ledB.Blink(50);     // Blink yellow led bar every 50 ms
    ledY.Blink(50);     // Blink blue led bar every 50 ms

    /* Keypad */
    if (KeyRising()) {
        switch (ch) {
            case 'A':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: A   DOMINATION");
                    selMode = MODE_A_DOMINATION;
                    confirmMode = true;
                }
                break;
            case 'B':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: B    JOINT OP.");
                    selMode = MODE_B_JOINT;
                    confirmMode = true;
                }
                break;
            case 'C':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: C      CLASSIC");
                    selMode = MODE_C_CLASSIC;
                    confirmMode = true;
                }
                break;
            case 'D':
                if (!confirmMode) {
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: D       POINTS");
                    selMode = MODE_D_POINTS;
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
                    ledB.Reset();
                    ledY.Reset();
                }
                break;
            case '#':
                if (confirmMode) {
                    selMode = MODE_SELECT;
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
 *  Mode Set Time loop function
 */
void ModeSetTime_Loop(void) {
    static int x;
    static bool confirmTime;
    // Select game time in minutes from keypad
    if (KeyRising()) {
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
                    switch (selMode) {
                        case MODE_A_DOMINATION:
                            mode = MODE_START_GAME;
                            lcd.setCursor(0, 0);
                            lcd.print("PRESS * AND #       ");
                            lcd.setCursor(0, 1);
                            lcd.print("TO START THE GAME   ");
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

/*
 *  Mode Set Code loop function
 */
void ModeSetCode_Loop(void) {
}

/*
 *  Mode Start Game loop function
 */
void ModeStartGame_Loop(void) {
    static bool astPressed;
    static bool cancPressed;
    static bool startCount;
    // Check double press of * and #
    ch = keypad.getChar();
    switch (ch) {
        case '*':
            astPressed = true;
            break;
        case '#':
            cancPressed = true;
            break;
        default:
            break;
    }
    if(astPressed && cancPressed) startCount = true;
    // Countdown to start
    if(startCount) {
        lcd.setCursor(0, 3);
        lcd.print("3 ");
        delay(1000);
        lcd.print("2 ");
        delay(1000);
        lcd.print("1 ");
        delay(1000);
        GameTimer.SetTime(gameTime_min*60);
        lcd.clear();
        // Display still configuration
        switch (selMode) {
            case MODE_A_DOMINATION:
                lcd.setCursor(0, 3);
                lcd.print("MODE: A   DOMINATION");
                break;
            case MODE_B_JOINT:
                lcd.setCursor(0, 3);
                lcd.print("MODE: B    JOINT OP.");
                break;
            case MODE_C_CLASSIC:
                lcd.setCursor(0, 3);
                lcd.print("MODE: C      CLASSIC");
                break;
            case MODE_D_POINTS:
                lcd.setCursor(0, 3);
                lcd.print("MODE: D       POINTS");
                break;
            default:
                break;
        }
        // Start the game
        mode = selMode;
    }
}

/*
 *  Mode A: Domination
 */
void ModeA_Loop(void) {

}

/*
 *  Mode B: Joint Operation
 */
void ModeB_Loop(void) {
}

/*
 *  Mode C: Classic
 */
void ModeC_Loop(void) {
}

/*
 *  Mode D: Points Game
 */
void ModeD_Loop(void) {
}

/******************************************************************************
* Main Arduino Functions (Setup + Loop)
*******************************************************************************/
/*
 *  Setup main function
 */
void setup() {
    // Init functions
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
            ModeSelect_Loop();
            break;
        case MODE_SET_TIME:
            ModeSetTime_Loop();
            break;
        case MODE_SET_CODE:
            ModeSetCode_Loop();
            break;
        case MODE_START_GAME:
            ModeStartGame_Loop();
            break;
        case MODE_A_DOMINATION:
            ModeA_Loop();
            break;
        case MODE_B_JOINT:
            ModeB_Loop();
            break;
        case MODE_C_CLASSIC:
            ModeC_Loop();
            break;
        case MODE_D_POINTS:
            ModeD_Loop();
            break;
        default:
            break;
    }
}

/* EOF */
