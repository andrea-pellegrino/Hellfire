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
/* Public libraries */
#include "Buzzer.h"                 // Buzzer Library
#include "I2CKeyPad.h"              // I2C Keypad Library
#include "LiquidCrystal_I2C.h"      // I2C LCD Screen Library
#include "Wire.h"                   // I2C Library

/* Private libraries */
#include "LedBar.h"                 // Led Bar Library
#include "PushButton.h"             // Pushbutton Library
#include "TimeCount.h"              // Time Counter Library

/******************************************************************************
* Defines
*******************************************************************************/
/* Software Version */
#define SW_VERSION      0
#define SW_REVISION     0

/* I2C Address */
#define I2C_ADR_KEY     0x20        // I2C Keypad Address (I2C Expansion PCF8574)
#define I2C_ADR_LCD     0x27        // I2C LCD Address

/* Pins Definition */
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
/* Game mode (main loop FSM) */
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
/* Keypad map */
const char keymap[19] = "123A456B789C*0#DNF";  // N = NoKey, F = Fail

/* Led maps */
const byte ledMapT1[5] = {GPIO_LED_B0, GPIO_LED_B1, GPIO_LED_B2, GPIO_LED_B3, GPIO_LED_B4};
const byte ledMapT2[5] = {GPIO_LED_Y0, GPIO_LED_Y1, GPIO_LED_Y2, GPIO_LED_Y3, GPIO_LED_Y4};

/* Instances */
LiquidCrystal_I2C lcd(I2C_ADR_LCD, 20, 4);  // I2C LCD 20x4 line display
I2CKeyPad keypad(I2C_ADR_KEY);              // I2C Keypad (with PCF8574)
LedBar ledT1(ledMapT1, 5);                  // Blue LED bar (team 1)
LedBar ledT2(ledMapT2, 5);                  // Yellow LED bar (team 2)
TimeCount GameTimer;                        // Timer of Game
PushButton swT1(GPIO_SW_RED);               // Red button (team 1)
PushButton swT2(GPIO_SW_GRN);               // Green button (team 2)
Buzzer buzzer(GPIO_BZR);                    // Buzzer

/* State variables */
mode_e mode = MODE_SELECT;
mode_e selMode = MODE_SELECT;

/* Keypad variables */
bool keyCurrPress = false;
bool keyLastPress = false;
char ch;

/* Time variables */
char gameTime_str[4];                       // Set digits (min)
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

/*
 *  GPIO Initialization
 */
void GPIO_Init(void) {

    /* Switch begin */
    //swT1.begin();       // Red button
    //swT2.begin();       // Green button

    /* Buzzer init */
    buzzer.begin(10);
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
    ledT1.blink(50);     // Blink (t1) blue led bar every 50 ms
    ledT2.blink(50);     // Blink (t2) yellow led bar every 50 ms

    /* Keypad */
    if (KeyRising()) {      // When rising edge
        switch (ch) {
            case 'A':
                if (!confirmMode) {     // Select mode A

                    /* Display confirm selection */
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: A   DOMINATION");

                    /* Next mode */
                    selMode = MODE_A_DOMINATION;
                    confirmMode = true;
                }
                break;

            case 'B':
                if (!confirmMode) {     // Select mode B

                    /* Display confirm selection */
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: B    JOINT OP.");

                    /* Next mode */
                    selMode = MODE_B_JOINT;
                    confirmMode = true;
                }
                break;

            case 'C':
                if (!confirmMode) {     // Select mode C

                    /* Display confirm selection */
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: C      CLASSIC");

                    /* Next mode */
                    selMode = MODE_C_CLASSIC;
                    confirmMode = true;
                }
                break;

            case 'D':
                if (!confirmMode) {     // Select mode D

                    /* Display confirm selection */
                    lcd.setCursor(0, 1);
                    lcd.print("CONFIRM SELECTION?");
                    lcd.setCursor(0, 3);
                    lcd.print("MODE: D       POINTS");

                    /* Next mode */
                    selMode = MODE_D_POINTS;
                    confirmMode = true;
                }
                break;

            case '*':
                if (confirmMode) {      // Confirm selected mode

                    /* Display set game time */
                    lcd.setCursor(0, 0);
                    lcd.print("SET GAME TIME (min):");
                    lcd.setCursor(0, 1);
                    lcd.print("           (max 999)");
                    lcd.setCursor(0, 3);
                    lcd.print("                    ");
                    lcd.setCursor(0, 1);
                    lcd.blink();

                    /* Reset bar lights */
                    ledT1.reset();
                    ledT2.reset();

                    /* Next mode */
                    mode = MODE_SET_TIME;       // Next state
                }
                break;

            case '#':                   // Cancel selected mode
                if (confirmMode) {

                    /* Display reset selected mode */
                    lcd.setCursor(0, 1);
                    lcd.print("                    ");
                    lcd.setCursor(0, 3);
                    lcd.print("                    ");

                    /* Next mode */
                    confirmMode = false;
                    selMode = MODE_SELECT;      // Reset selected mode
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
    static int x;                // cursor position
    static bool confirmTime;     // 0-> set time 1-> confirm time

    /* Keypad */
    if (KeyRising()) {      // When rising edge
        switch (ch) {
            case '0'...'9':

                /* Write ditigs (max 3 digits) */
                if ((!confirmTime) && (x >= 0) && (x <= 2)) {

                    /* Display print digit */
                    lcd.setCursor(x, 1);
                    lcd.print(ch);

                    /* Save digit into string */
                    gameTime_str[x] = ch;
                    x++;
                }
                break;

            case '*':

                /* Set time */
                if ((!confirmTime) && (x > 0)) {

                    /* Convert digits string into int */
                    gameTime_min = atoi(gameTime_str);

                    /* Display confirm time*/
                    lcd.noBlink();
                    lcd.setCursor(0, 2);
                    lcd.print("CONFIRM?            ");

                    /* Next mode */
                    confirmTime = true;
                }
                /* Confirm time */
                else if (confirmTime) {

                    /* Reset display */
                    lcd.clear();

                    /* Go to selected game */
                    switch (selMode) {

                        case MODE_A_DOMINATION:                 // Game A

                            /* Start game */
                            lcd.setCursor(0, 0);
                            lcd.print("PRESS * AND #       ");
                            lcd.setCursor(0, 1);
                            lcd.print("TO START THE GAME   ");

                            /* Next mode (go to start game, no code is needed) */
                            mode = MODE_START_GAME;
                            break;

                        case MODE_B_JOINT:                      // Game B
                            break;

                        case MODE_C_CLASSIC:                    // Game C
                            break;

                        case MODE_D_POINTS:                     // Game D
                            break;

                        default:
                            break;
                    }
                }
                break;

            case '#':

                /* Delete ditigs (max 3 digits) */
                if ((!confirmTime) && (x >= 1) && (x <= 3)) {

                    /* Display remove digit */
                    x--;
                    lcd.setCursor(x, 1);
                    lcd.print(" ");
                    lcd.setCursor(x, 1);

                    /* Remove digit from string */
                    gameTime_str[x] = '\0';
                }

                /* Do not confirm time set */
                else if (confirmTime) {

                    /* D */
                    lcd.setCursor(0, 2);
                    lcd.print("                    ");
                    lcd.setCursor(x, 1);
                    lcd.blink();

                    /* Next mode */
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

    /* Keypad */
    ch = keypad.getChar();
    switch (ch) {           // Check double press of * and #
        case '*':
            astPressed = true;
            break;

        case '#':
            cancPressed = true;
            break;

        default:
            break;
    }

    /* Start countdown trigger */
    if(astPressed && cancPressed) startCount = true;

    /* Countdown */
    if(startCount) {

        /* Display show countdown */
        lcd.setCursor(0, 3);
        lcd.print("3 ");
        delay(1000);
        lcd.print("2 ");
        delay(1000);
        lcd.print("1 ");
        delay(1000);
        lcd.clear();

        /* Set game time counter */
        GameTimer.setTime(gameTime_min*60);

        /* Display still configuration */
        switch (selMode) {

            case MODE_A_DOMINATION:                 // A

                /* Display show current game init */
                lcd.setCursor(0, 2);
                lcd.print("Countdown:");
                lcd.setCursor(12, 2);
                lcd.print(GameTimer.str());
                lcd.setCursor(0, 3);
                lcd.print("MODE: A   DOMINATION");
                break;

            case MODE_B_JOINT:                      // B

                /* Display show current game */
                lcd.setCursor(0, 3);
                lcd.print("MODE: B    JOINT OP.");
                break;

            case MODE_C_CLASSIC:                    // C

                /* Display show current game */
                lcd.setCursor(0, 3);
                lcd.print("MODE: C      CLASSIC");
                break;

            case MODE_D_POINTS:                     // D

                /* Display show current game */
                lcd.setCursor(0, 3);
                lcd.print("MODE: D       POINTS");
                break;

            default:
                break;
        }

        /* Next mode (start the game) */
        mode = selMode;
    }
}

/*
 *  Mode A: Domination
 */
void ModeA_Loop(void) {

    if (GameTimer.count()) {
        GameTimer.dec();
        lcd.setCursor(12, 2);
        lcd.print(GameTimer.str());
    }
    if(swT1.isPressed()) ledT1.inc();
    else ledT1.dec();
    if(swT2.isPressed()) ledT2.inc();
    else ledT2.dec();

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

    /* I2c init */
    I2C_Init();

    /* Liquidcrystal I2C init */
    LCD_Init();

    /* Keypad I2C with PCF8574 init */
    Keypad_Init();

    /* GPIO init (Switch - Buzzer) */
    GPIO_Init();
}

/*
 *  Loop main function
 */
void loop() {

    /* Mode Finite State Machine */
    switch (mode) {

        case MODE_SELECT:           // Select mode
            ModeSelect_Loop();
            break;

        case MODE_SET_TIME:         // Set game time
            ModeSetTime_Loop();
            break;

        case MODE_SET_CODE:         // Set code for unlock
            ModeSetCode_Loop();
            break;

        case MODE_START_GAME:       // Start the game
            ModeStartGame_Loop();
            break;

        case MODE_A_DOMINATION:     // GAME MODE: A
            ModeA_Loop();
            break;

        case MODE_B_JOINT:          // GAME MODE: B
            ModeB_Loop();
            break;

        case MODE_C_CLASSIC:        // GAME MODE: C
            ModeC_Loop();
            break;

        case MODE_D_POINTS:         // GAME MODE: D
            ModeD_Loop();
            break;

        default:
            break;
    }


}

/* EOF */
