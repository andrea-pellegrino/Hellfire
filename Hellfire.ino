/*******************************************************************************
* Title                 :   Project Hellfire
* Filename              :   Hellfire.ino
* Author                :   Andrea Pellegrino
* Origin Date           :   17/07/2022
* Version               :   0.3
* Compiler              :   avr-gcc
* Target                :   Arduino UNO (avr)
* Notes                 :   using arduino-cli
*******************************************************************************/
/*
 * Description:
 * Game Bomb SW for SoftAir - Arduino UNO
 *
 * Version log:
 * 0.0: First draft, mode A (Domination) implemented            - 16.10.2022
 * 0.1: mode B (Joing Operation) implemented                    - 23.10.2022
 * 0.2: mode C (Classic) implemented                            - 31.10.2022
 * 0.3: mode D (Points) implemented                             - 01.11.2022
 */

/******************************************************************************
* Includes
*******************************************************************************/
/* Public libraries */
#include "Button.h"                 // Button Library
#include "Buzzer.h"                 // Buzzer Library
#include "I2CKeyPad.h"              // I2C Keypad Library
#include "LiquidCrystal_I2C.h"      // I2C LCD Screen Library
#include "Wire.h"                   // I2C Library

/* Private libraries */
#include "LedBar.h"                 // Led Bar Library
#include "TimeCount.h"              // Time Counter Library

/******************************************************************************
* Defines
*******************************************************************************/
/* Software Version */
#define SW_VERSION      0
#define SW_REVISION     3

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

#define MINUTE_S        60

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

/* Game advantage */
enum adv_e {
    ADV_NONE_NOPRESS,       // Mode: A, B, C, D
    ADV_NONE_T1PRESS,       // Mode: A, B, C, D
    ADV_NONE_T2PRESS,       // Mode: A, B, D
    ADV_T1_NOPRESS,         // Mode: A, D
    ADV_T1_T2PRESS,         // Mode: A, D
    ADV_T2_NOPRESS,         // Mode: A, D
    ADV_T2_T1PRESS,         // Mode: A, D
    ADV_T1T2_NOPRESS,       // Mode: D
    ADV_INS_PASSWORD,       // Mode: B, C
    ADV_END_GAME            // Mode: A, B, C, D
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
TimeCount gameTim;                          // Timer of Game
TimeCount ledT1Tim;                         // Timer of led bar (team 1)
TimeCount ledT2Tim;                         // Timer of led bar (team 2)
TimeCount t1Tim;                            // Game timer of team 1
TimeCount t2Tim;                            // Game timer of team 2
Button swT1(GPIO_SW_RED);                   // Red button (team 1)
Button swT2(GPIO_SW_GRN);                   // Green button (team 2)
Buzzer buzzer(GPIO_BZR);                    // Buzzer

/* State variables */
mode_e mode = MODE_SELECT;
mode_e selMode = MODE_SELECT;
adv_e adv = ADV_NONE_NOPRESS;

/* Keypad variables */
bool keyCurrPress = false;
bool keyLastPress = false;
char ch;

/* Ledbar variables */
int ledT1State;
int ledT2State;

/* Points */
int pointsT1;
int pointsT2;
char points_str[6];

/* Time variables */
char gameTime_str[4];                       // Set digits (min)
unsigned int gameTime_min;
unsigned int t1Time_s;
unsigned int t2Time_s;
unsigned int gameTime_s;

/* Code/Password */
char codeT1_str[6];
char codeT2_str[6];
char codeSet_str[6];
long codeT1_long;
long codeT2_long;
long codeSet_long;

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
 *  Keypad I2C Initialization + Button init
 */
void Keypad_Init(void) {

    /* Keypad begin */
    keypad.begin();

    /* Keypad load key map */
    keypad.loadKeyMap(keymap);

    /* Button begin */
    swT1.begin();
    swT2.begin();
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

                            /* Display set game time */
                            lcd.setCursor(0, 0);
                            lcd.print("SET PASSWORD TEAM 1:");
                            lcd.setCursor(0, 1);
                            lcd.print("          (5 digits)");
                            lcd.setCursor(0, 1);
                            lcd.blink();

                            /* Next mode */
                            mode = MODE_SET_CODE;
                            break;

                        case MODE_C_CLASSIC:                    // Game C

                            /* Display set game time */
                            lcd.setCursor(0, 0);
                            lcd.print("SET PASSWORD:       ");
                            lcd.setCursor(0, 1);
                            lcd.print("          (5 digits)");
                            lcd.setCursor(0, 1);
                            lcd.blink();

                            /* Next mode */
                            mode = MODE_SET_CODE;
                            break;

                        case MODE_D_POINTS:                     // Game D

                            /* Start game */
                            lcd.setCursor(0, 0);
                            lcd.print("PRESS * AND #       ");
                            lcd.setCursor(0, 1);
                            lcd.print("TO START THE GAME   ");

                            /* Next mode (go to start game, no code is needed) */
                            mode = MODE_START_GAME;
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
    static int x;                // cursor position
    static bool fTeam;           // 0-> team 1   1-> team 2

    /* Keypad */
    if (KeyRising()) {      // When rising edge
        switch (ch) {
            case '0'...'9':

                /* Write ditigs (max 5 digits) */
                if ((x >= 0) && (x <= 4)) {

                    /* Display print digit */
                    lcd.setCursor(x, 1);
                    lcd.print(ch);

                    /* Save digit into string */
                    if(!fTeam) codeT1_str[x] = ch;
                    else codeT2_str[x] = ch;
                    x++;

                    if (x == 5) {

                        /* Display confirm code*/
                        lcd.noBlink();
                        lcd.setCursor(0, 2);
                        lcd.print("CONFIRM?            ");
                    }                    /* Reset display */
                }
                break;

            case '*':

                if (x == 5) {

                    /* Convert digits string into int */
                    if(!fTeam) codeT1_long = atol(codeT1_str);
                    else codeT2_long = atol(codeT2_str);

                    /* Go to selected game */
                    switch (selMode) {

                        case MODE_A_DOMINATION:                 // Game A
                            break;

                        case MODE_B_JOINT:                      // Game B

                            if(!fTeam) {

                                /* Display set game time */
                                lcd.setCursor(0, 0);
                                lcd.print("SET PASSWORD TEAM 2:");
                                lcd.setCursor(0, 1);
                                lcd.print("          (5 digits)");
                                lcd.setCursor(0, 2);
                                lcd.print("                    ");
                                lcd.setCursor(0, 1);
                                lcd.blink();
                                x = 0;      // Reset cursor

                                /* Set second code */
                                fTeam = true;

                            }
                            else {

                                /* Reset display */
                                lcd.clear();

                                /* Start game */
                                lcd.setCursor(0, 0);
                                lcd.print("PRESS * AND #       ");
                                lcd.setCursor(0, 1);
                                lcd.print("TO START THE GAME   ");

                                /* Next mode (go to start game) */
                                mode = MODE_START_GAME;
                            }
                            break;

                        case MODE_C_CLASSIC:                    // Game C

                            /* Reset display */
                            lcd.clear();

                            /* Start game */
                            lcd.setCursor(0, 0);
                            lcd.print("PRESS * AND #       ");
                            lcd.setCursor(0, 1);
                            lcd.print("TO START THE GAME   ");

                            /* Next mode (go to start game) */
                            mode = MODE_START_GAME;
                            break;

                        case MODE_D_POINTS:                     // Game D
                            break;

                        default:
                            break;
                    }
                }
                break;

            case '#':

                /* Delete ditigs (max 5 digits) */
                if ((x >= 1) && (x <= 5)) {

                    /* Reset Confirm */
                    lcd.setCursor(0, 2);
                    lcd.print("                    ");

                    /* Display remove digit */
                    x--;
                    lcd.setCursor(x, 1);
                    lcd.print(" ");
                    lcd.setCursor(x, 1);
                    lcd.blink();

                    /* Remove digit from string */
                    if(!fTeam) codeT1_str[x] = '\0';
                    else codeT2_str[x]= '\0';
                }
                break;

            default:
                break;
        }
    }
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

        /* Set time counters */
        gameTim.setTime(gameTime_min*60);
        t1Tim.setTime(t1Time_s);
        t2Tim.setTime(t2Time_s);


        /* Display still configuration */
        switch (selMode) {

            case MODE_A_DOMINATION:                 // A

                /* Display show current game init */
                lcd.setCursor(0, 0);
                lcd.print("Team 1:");
                lcd.setCursor(12, 0);
                lcd.print(t1Tim.str());
                lcd.setCursor(0, 1);
                lcd.print("Team 2:");
                lcd.setCursor(12, 1);
                lcd.print(t2Tim.str());
                lcd.setCursor(0, 2);
                lcd.print("Countdown:");
                lcd.setCursor(12, 2);
                lcd.print(gameTim.str());

                /* Display show current game */
                lcd.setCursor(0, 3);
                lcd.print("MODE: A   DOMINATION");
                break;

            case MODE_B_JOINT:                      // B

                /* Display show current game init */
                lcd.setCursor(12, 2);
                lcd.print(gameTim.str());

                /* Display show current game */
                lcd.setCursor(0, 3);
                lcd.print("MODE: B    JOINT OP.");
                break;

            case MODE_C_CLASSIC:                    // C

                /* Display show current game init */
                lcd.setCursor(12, 2);
                lcd.print(gameTim.str());

                /* Display show current game */
                lcd.setCursor(0, 3);
                lcd.print("MODE: C      CLASSIC");
                break;

            case MODE_D_POINTS:                     // D

            /* Display show current game init */
                lcd.setCursor(0, 0);
                lcd.print("Team 1 points: 0    ");
                lcd.setCursor(0, 1);
                lcd.print("Team 2 points: 0    ");
                lcd.setCursor(0, 2);
                lcd.print("Countdown:");
                lcd.setCursor(12, 2);
                lcd.print(gameTim.str());

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

    /* Game time counter */
    if ((gameTim.count()) && (adv != ADV_END_GAME)) {
        gameTime_s = gameTim.dec();
        lcd.setCursor(12, 2);
        lcd.print(gameTim.str());

        /* Game over */
        if (gameTime_s == 0) {
            lcd.clear();
            adv = ADV_END_GAME;
        }
    }

    /* Advantage */
    switch (adv) {
        case ADV_NONE_NOPRESS:

            /* Team 1 button is pressed */
            if (swT1.read() == LOW) {
                ledT1Tim.setTime(0);
                adv = ADV_NONE_T1PRESS;
            }

            /* Team 2 button is pressed */
            if (swT2.read() == LOW) {
                ledT2Tim.setTime(0);
                adv = ADV_NONE_T2PRESS;
            }
            break;

        case ADV_NONE_T1PRESS:

            /* Team 1 Led Bar increases */
            if (swT1.read() == LOW) {
                if (ledT1Tim.count()) {
                    ledT1State = ledT1Tim.inc();
                    ledT1.inc();
                }
                if(ledT1State == 5) {
                    t1Tim.setTime(t1Time_s);
                    adv = ADV_T1_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT1.reset();
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_NONE_T2PRESS:

            /* Team 2 Led Bar increases */
            if (swT2.read() == LOW) {
                if (ledT2Tim.count()) {
                    ledT2State = ledT2Tim.inc();
                    ledT2.inc();
                }
                if(ledT2State == 5) {
                    t2Tim.setTime(t2Time_s);
                    adv = ADV_T2_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT2.reset();
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_T1_NOPRESS:

            /* T1 Timer is increasing */
            if (t1Tim.count()) {
                t1Time_s = t1Tim.inc();
                lcd.setCursor(12, 0);
                lcd.print(t1Tim.str());
            }

            /* Team 2 button is pressed */
            if (swT2.read() == LOW) {
                ledT1Tim.setTime(5);
                adv = ADV_T1_T2PRESS;
            }
            break;

        case ADV_T1_T2PRESS:

            /* T1 Timer is increasing */
            if (t1Tim.count()) {
                t1Time_s = t1Tim.inc();
                lcd.setCursor(12, 0);
                lcd.print(t1Tim.str());
            }

            /* Team 1 Led Bar decreases */
            if (swT2.read() == LOW) {
                if (ledT1Tim.count()) {
                    ledT1State = ledT1Tim.dec();
                    ledT1.dec();
                }
                if(ledT1State == 0) {
                    adv = ADV_NONE_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT1.full();
                adv = ADV_T1_NOPRESS;
            }
            break;

        case ADV_T2_NOPRESS:

            /* T2 Timer is increasing */
            if (t2Tim.count()) {
                t2Time_s = t2Tim.inc();
                lcd.setCursor(12, 1);
                lcd.print(t2Tim.str());
            }

            /* Team 1 button is pressed */
            if (swT1.read() == LOW) {
                ledT2Tim.setTime(5);
                adv = ADV_T2_T1PRESS;
            }
            break;

        case ADV_T2_T1PRESS:

            /* T2 Timer is increasing */
            if (t2Tim.count()) {
                t2Time_s = t2Tim.inc();
                lcd.setCursor(12, 1);
                lcd.print(t2Tim.str());
            }

            /* Team 2 Led Bar decreases */
            if (swT1.read() == LOW) {
                if (ledT2Tim.count()) {
                    ledT2State = ledT2Tim.dec();
                    ledT2.dec();
                }
                if(ledT2State == 0) {
                    adv = ADV_NONE_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT2.full();
                adv = ADV_T2_NOPRESS;
            }
            break;

        case ADV_END_GAME:

            /* Team 1 wins */
            if (t1Time_s > t2Time_s) {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   TEAM 1 WINS    *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(50);
                ledT2.reset();
            }
            /* Team 2 wins */
            else if (t2Time_s > t1Time_s) {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   TEAM 2 WINS    *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.reset();
                ledT2.blink(50);
            }
            /* Draw */
            else {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*       DRAW       *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(50);
                ledT2.blink(50);
            }
            break;

        default:
            break;
    }
}

/*
 *  Mode B: Joint Operation
 */
void ModeB_Loop(void) {
    static int x;            // cursor position
    static bool fTeam;       // 0-> team 1   1-> team 2
    static int att;          // n° of attemps

    /* Game time counter */
    if ((gameTim.count()) && (adv != ADV_END_GAME)) {
        gameTime_s = gameTim.dec();
        lcd.setCursor(12, 2);
        lcd.print(gameTim.str());
        lcd.setCursor(x, 1);

        /* Game over */
        if (gameTime_s == 0) {
            lcd.clear();
            adv = ADV_END_GAME;
        }
    }

    /* Advantage */
    switch (adv) {
        case ADV_NONE_NOPRESS:

            /* Team 1 button is pressed and T1 not done */
            if ((ledT1State == 0) && (swT1.read() == LOW)) {
                ledT1Tim.setTime(0);
                adv = ADV_NONE_T1PRESS;
            }

            /* Team 2 button is pressed and T2 not done */
            if ((ledT2State == 0) && (swT2.read() == LOW)) {
                ledT2Tim.setTime(0);
                adv = ADV_NONE_T2PRESS;
            }

            /* If both T1 and T2 are done, end of game */
            if ((ledT1State == 5) && (ledT2State == 5)) {
                lcd.clear();
                adv = ADV_END_GAME;
            }
            break;

        case ADV_NONE_T1PRESS:

            /* Team 1 Led Bar increases */
            if (swT1.read() == LOW) {
                if (ledT1Tim.count()) {
                    ledT1State = ledT1Tim.inc();
                    ledT1.inc();
                }
                if(ledT1State == 5) {
                    t1Tim.setTime(t1Time_s);
                    adv = ADV_INS_PASSWORD;
                    fTeam = false;       // 1st team
                    lcd.setCursor(0, 0);
                    lcd.print("TEAM 1 INS PASSWORD ");
                    lcd.setCursor(0, 2);
                    lcd.print("3 REM.");
                    lcd.setCursor(0, 1);
                    lcd.blink();
                }
            }
            /* Button released early */
            else {
                ledT1.reset();
                ledT1State = 0;
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_NONE_T2PRESS:

            /* Team 2 Led Bar increases */
            if (swT2.read() == LOW) {
                if (ledT2Tim.count()) {
                    ledT2State = ledT2Tim.inc();
                    ledT2.inc();
                }
                if(ledT2State == 5) {
                    t2Tim.setTime(t2Time_s);
                    adv = ADV_INS_PASSWORD;
                    fTeam = true;       // 2nd team
                    lcd.setCursor(0, 0);
                    lcd.print("TEAM 2 INS PASSWORD ");
                    lcd.setCursor(0, 2);
                    lcd.print("3 REM.");
                    lcd.setCursor(0, 1);
                    lcd.blink();
                }
            }
            /* Button released early */
            else {
                ledT2.reset();
                ledT2State = 0;
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_INS_PASSWORD:

            /* Keypad */
            if (KeyRising()) {      // When rising edge
                switch (ch) {
                    case '0'...'9':

                        /* Write ditigs (max 5 digits) */
                        if ((x >= 0) && (x <= 4)) {

                            /* Display print digit */
                            lcd.setCursor(x, 1);
                            lcd.print(ch);

                            /* Save digit into string */
                            codeSet_str[x] = ch;
                            x++;

                            if (x == 5) {

                                /* Display confirm code*/
                                lcd.noBlink();
                                lcd.setCursor(7, 0);
                                lcd.print("CONFIRM?     ");
                            }
                        }
                        break;

                    case '*':

                        if (x == 5) {

                            /* Convert digits string into int */
                            codeSet_long = atol(codeSet_str);

                            /* If code is correct */
                            if(((codeSet_long == codeT1_long) && (!fTeam)) ||   // Team 1
                               ((codeSet_long == codeT2_long) && (fTeam))) {    // Team 2
                                att = 0;
                                x = 0;

                                /* Reset text */
                                lcd.setCursor(0, 0);
                                lcd.print("                    ");
                                lcd.setCursor(0, 1);
                                lcd.print("                    ");
                                lcd.setCursor(0, 2);
                                lcd.print("      ");
                                adv = ADV_NONE_NOPRESS;
                            }
                            else {
                                att++;
                                x = 0;

                                /* Reset Code */
                                lcd.setCursor(0, 1);
                                lcd.print("       ");

                                /* Change remaining attemps */
                                lcd.setCursor(0, 2);
                                if (att == 1) lcd.print("2 REM.");
                                else if (att == 2) lcd.print("1 REM.");

                                /* Reset Confirm */
                                lcd.setCursor(7, 0);
                                lcd.print("INS PASSWORD ");
                                lcd.setCursor(0, 1);
                                lcd.blink();
                            }
                        }
                        break;

                    case '#':

                        /* Delete ditigs (max 5 digits) */
                        if ((x >= 1) && (x <= 5)) {

                            /* Reset Confirm */
                            lcd.setCursor(7, 0);
                            lcd.print("INS PASSWORD ");

                            /* Display remove digit */
                            x--;
                            lcd.setCursor(x, 1);
                            lcd.print(" ");
                            lcd.setCursor(x, 1);
                            lcd.blink();

                            /* Remove digit from string */
                            codeSet_str[x] = ch;
                        }
                        break;

                    default:
                        break;
                }
            }

            /* If attemps are 3, end of game */
            if (att == 3) {
                lcd.clear();
                adv = ADV_END_GAME;
            }

            break;

        case ADV_END_GAME:

            /* Bomb disarmed */
            if ((ledT1State == 5) && (ledT2State == 5)) {
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   BOMB DISARMED  *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(500);
                ledT2.blink(500);
            }

            /* Bomb exploded */
            else {
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   BOMB EXPLODED  *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(50);
                ledT2.blink(50);
            }

            break;

        default:
            break;
    }
}

/*
 *  Mode C: Classic
 */
void ModeC_Loop(void) {
    static int x;            // cursor position
    static int att;          // n° of attemps

    /* Game time counter */
    if ((gameTim.count()) && (adv != ADV_END_GAME)) {
        gameTime_s = gameTim.dec();
        lcd.setCursor(12, 2);
        lcd.print(gameTim.str());
        lcd.setCursor(x, 1);

        /* Game over */
        if (gameTime_s == 0) {
            lcd.clear();
            adv = ADV_END_GAME;
        }
    }

    /* Advantage */
    switch (adv) {
        case ADV_NONE_NOPRESS:

            /* Team 1 & 2 button are pressed and T1 not done */
            if ((ledT1State == 0) && (swT1.read() == LOW) && (swT2.read() == LOW)) {
                ledT1Tim.setTime(0);
                adv = ADV_NONE_T1PRESS;
            }

            /* If T1 is done, end of game */
            if (ledT1State == 5) {
                lcd.clear();
                adv = ADV_END_GAME;
            }
            break;

        case ADV_NONE_T1PRESS:

            /* Team 1-2 Led Bars increases */
            if ((swT1.read() == LOW) && (swT2.read() == LOW)) {
                if (ledT1Tim.count()) {
                    ledT1State = ledT1Tim.inc();
                    ledT1.inc();
                    ledT2.inc();
                }
                if(ledT1State == 5) {
                    t1Tim.setTime(t1Time_s);
                    adv = ADV_INS_PASSWORD;
                    lcd.setCursor(0, 0);
                    lcd.print("INSERT PASSWORD ");
                    lcd.setCursor(0, 2);
                    lcd.print("3 REM.");
                    lcd.setCursor(0, 1);
                    lcd.blink();
                }
            }
            /* Button released early */
            else {
                ledT1.reset();
                ledT2.reset();
                ledT1State = 0;
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_INS_PASSWORD:

            /* Keypad */
            if (KeyRising()) {      // When rising edge
                switch (ch) {
                    case '0'...'9':

                        /* Write ditigs (max 5 digits) */
                        if ((x >= 0) && (x <= 4)) {

                            /* Display print digit */
                            lcd.setCursor(x, 1);
                            lcd.print(ch);

                            /* Save digit into string */
                            codeSet_str[x] = ch;
                            x++;

                            if (x == 5) {

                                /* Display confirm code*/
                                lcd.noBlink();
                                lcd.setCursor(7, 0);
                                lcd.print("CONFIRM?     ");
                            }
                        }
                        break;

                    case '*':

                        if (x == 5) {

                            /* Convert digits string into int */
                            codeSet_long = atol(codeSet_str);

                                /* Code is correct */
                                if(codeSet_long == codeT1_long) {
                                    att = 0;
                                    x = 0;

                                    /* Reset text */
                                    lcd.setCursor(0, 0);
                                    lcd.print("                    ");
                                    lcd.setCursor(0, 1);
                                    lcd.print("                    ");
                                    lcd.setCursor(0, 2);
                                    lcd.print("      ");
                                    adv = ADV_NONE_NOPRESS;
                                }
                                else {
                                    att++;
                                    x = 0;

                                    /* Reset Code */
                                    lcd.setCursor(0, 1);
                                    lcd.print("       ");

                                    /* Change remaining attemps */
                                    lcd.setCursor(0, 2);
                                    if (att == 1) lcd.print("2 REM.");
                                    else if (att == 2) lcd.print("1 REM.");

                                    /* Reset Confirm */
                                    lcd.setCursor(7, 0);
                                    lcd.print("PASSWORD     ");
                                    lcd.setCursor(0, 1);
                                    lcd.blink();
                                }
                            }
                        break;

                    case '#':

                        /* Delete ditigs (max 5 digits) */
                        if ((x >= 1) && (x <= 5)) {

                            /* Reset Confirm */
                            lcd.setCursor(7, 0);
                            lcd.print("PASSWORD     ");

                            /* Display remove digit */
                            x--;
                            lcd.setCursor(x, 1);
                            lcd.print(" ");
                            lcd.setCursor(x, 1);
                            lcd.blink();

                            /* Remove digit from string */
                            codeSet_str[x] = ch;
                        }
                        break;

                    default:
                        break;
                }
            }

            /* If attemps are 3, end of game */
            if (att == 3) {
                lcd.clear();
                adv = ADV_END_GAME;
            }

            break;

        case ADV_END_GAME:

            /* Bomb disarmed */
            if (ledT1State == 5) {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   BOMB DISARMED  *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(500);
                ledT2.blink(500);
            }

            /* Bomb exploded */
            else {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   BOMB EXPLODED  *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(50);
                ledT2.blink(50);
            }

            break;

        default:
            break;
    }
}

/*
 *  Mode D: Points Game
 */
void ModeD_Loop(void) {
    static int att;          // n° of attemps

    /* Game time counter */
    if ((gameTim.count()) && (adv != ADV_END_GAME)) {
        gameTime_s = gameTim.dec();
        lcd.setCursor(12, 2);
        lcd.print(gameTim.str());

        /* Game over */
        if (gameTime_s == 0) {
            lcd.clear();
            adv = ADV_END_GAME;
        }
    }

    /* Advantage */
    switch (adv) {
        case ADV_NONE_NOPRESS:

            /* Team 1 button is pressed */
            if (swT1.read() == LOW) {
                ledT1Tim.setTime(0);
                adv = ADV_NONE_T1PRESS;
            }

            /* Team 2 button is pressed */
            if (swT2.read() == LOW) {
                ledT2Tim.setTime(0);
                adv = ADV_NONE_T2PRESS;
            }
            break;

        case ADV_NONE_T1PRESS:

            /* Team 1 Led Bar increases */
            if (swT1.read() == LOW) {
                if (ledT1Tim.count()) {
                    ledT1State = ledT1Tim.inc();
                    ledT1.inc();
                }
                if(ledT1State == 5) {
                    pointsT1++;             // 1 point for Team 1
                    snprintf(points_str, 5, "%d", pointsT1);
                    lcd.setCursor(15, 0);
                    lcd.print(points_str);
                    t1Tim.setTime(0);      // Reset 1 minute timer
                    t1Time_s = 0;
                    adv = ADV_T1_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT1.reset();
                ledT1State = 0;
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_NONE_T2PRESS:

            /* Team 2 Led Bar increases */
            if (swT2.read() == LOW) {
                if (ledT2Tim.count()) {
                    ledT2State = ledT2Tim.inc();
                    ledT2.inc();
                }
                if(ledT2State == 5) {
                    pointsT2++;             // 1 point for Team 2
                    snprintf(points_str, 5, "%d", pointsT2);
                    lcd.setCursor(15, 1);
                    lcd.print(points_str);
                    t2Tim.setTime(0);      // Set 1 minute timer
                    t2Time_s = 0;
                    adv = ADV_T2_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT2.reset();
                ledT2State = 0;
                adv = ADV_NONE_NOPRESS;
            }
            break;

        case ADV_T1_NOPRESS:

            /* T1 Timer is increasing */
            if (t1Tim.count()) {
                t1Time_s = t1Tim.inc();
            }

            /* T1 Timer is over */
            if (t1Time_s >= MINUTE_S) {
                ledT1.reset();
                ledT1State = 0;
                adv = ADV_NONE_NOPRESS;
            }

            /* Team 2 button is pressed */
            if (swT2.read() == LOW) {
                ledT2Tim.setTime(0);
                adv = ADV_T1_T2PRESS;
            }
            break;

        case ADV_T1_T2PRESS:

            /* T1 Timer is increasing */
            if (t1Tim.count()) {
                t1Time_s = t1Tim.inc();
            }

            /* Team 2 Led Bar increases */
            if (swT2.read() == LOW) {
                if (ledT2Tim.count()) {
                    ledT2State = ledT2Tim.inc();
                    ledT2.inc();
                }
                if(ledT2State == 5) {
                    pointsT2++;             // 1 point for Team 2
                    snprintf(points_str, 5, "%d", pointsT2);
                    lcd.setCursor(15, 1);
                    lcd.print(points_str);
                    t2Tim.setTime(0);      // Set 1 minute timer
                    t2Time_s = 0;
                    adv = ADV_T1T2_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT2.reset();
                ledT2State = 0;
                adv = ADV_T1_NOPRESS;
            }
            break;

        case ADV_T2_NOPRESS:

            /* T2 Timer is increasing */
            if (t2Tim.count()) {
                t2Time_s = t2Tim.inc();
            }

            /* T2 Timer is over */
            if (t2Time_s >= MINUTE_S) {
                ledT2.reset();
                ledT2State = 0;
                adv = ADV_NONE_NOPRESS;
            }

            /* Team 1 button is pressed */
            if (swT1.read() == LOW) {
                ledT1Tim.setTime(0);
                adv = ADV_T2_T1PRESS;
            }
            break;

        case ADV_T2_T1PRESS:

            /* T2 Timer is increasing */
            if (t2Tim.count()) {
                t2Time_s = t2Tim.inc();
            }

            /* Team 1 Led Bar increases */
            if (swT1.read() == LOW) {
                if (ledT1Tim.count()) {
                    ledT1State = ledT1Tim.inc();
                    ledT1.inc();
                }
                if(ledT1State == 5) {
                    pointsT1++;             // 1 point for Team 1
                    snprintf(points_str, 5, "%d", pointsT1);
                    lcd.setCursor(15, 0);
                    lcd.print(points_str);
                    t1Tim.setTime(0);      // Set 1 minute timer
                    t1Time_s = 0;
                    adv = ADV_T1T2_NOPRESS;
                }
            }
            /* Button released early */
            else {
                ledT1.reset();
                ledT1State = 0;
                adv = ADV_T2_NOPRESS;
            }
            break;

        case ADV_T1T2_NOPRESS:

            /* T1 Timer is increasing */
            if (t1Tim.count()) {
                t1Time_s = t1Tim.inc();
            }

            /* T2 Timer is increasing */
            if (t2Tim.count()) {
                t2Time_s = t2Tim.inc();
            }

            /* T1 Timer is over */
            if (t1Time_s >= MINUTE_S) {
                ledT1.reset();
                ledT1State = 0;
                adv = ADV_T2_NOPRESS;
            }

            /* T2 Timer is over */
            if (t2Time_s >= MINUTE_S) {
                ledT2.reset();
                ledT2State = 0;
                adv = ADV_T1_NOPRESS;
            }
            break;

        case ADV_END_GAME:

            /* Team 1 wins */
            if (pointsT1 > pointsT2) {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   TEAM 1 WINS    *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(50);
                ledT2.reset();
            }
            /* Team 2 wins */
            else if (pointsT2 > pointsT1) {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*   TEAM 2 WINS    *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.reset();
                ledT2.blink(50);
            }
            /* Draw */
            else {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("*       DRAW       *");
                lcd.setCursor(0, 2);
                lcd.print("*                  *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                ledT1.blink(50);
                ledT2.blink(50);
            }
            break;

        default:
            break;
    }
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
