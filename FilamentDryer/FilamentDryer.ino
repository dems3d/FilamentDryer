#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LCDMenuLib2.h>

#define _LCDML_DISP_cols  16
#define _LCDML_DISP_rows  2

#define _LCDML_DISP_cfg_cursor                     0x7E   // cursor Symbol
#define _LCDML_DISP_cfg_scrollbar                  1      // enable a scrollbar

LiquidCrystal_I2C lcd(0x27, _LCDML_DISP_cols, _LCDML_DISP_rows);

const uint8_t scroll_bar[5][8] = {
  {B10001, B10001, B10001, B10001, B10001, B10001, B10001, B10001}, // scrollbar top
  {B11111, B11111, B10001, B10001, B10001, B10001, B10001, B10001}, // scroll state 1
  {B10001, B10001, B10001, B10001, B10001, B10001, B11111, B11111}  // scrollbar bottom
};


void lcdml_menu_display();
void lcdml_menu_clear();
void lcdml_menu_control();

LCDMenuLib2_menu LCDML_0 (255, 0, 0, NULL, NULL); // root menu element (do not change)
LCDMenuLib2 LCDML(LCDML_0, _LCDML_DISP_rows, _LCDML_DISP_cols, lcdml_menu_display, lcdml_menu_clear, lcdml_menu_control);

LCDML_add         (0  , LCDML_0           , 1  , "Start"            , mFunc_start);       // this menu function can be found on "LCDML_display_menuFunction" tab
LCDML_add         (1  , LCDML_0           , 2  , "Set box temp"     , mFunc_setBoxTemp);        // this menu function can be found on "LCDML_display_menuFunction" tab
LCDML_add         (2  , LCDML_0           , 3  , "Set time"         , mFunc_setTime);        // this menu function can be found on "LCDML_display_menuFunction" tab
LCDML_add         (3  , LCDML_0           , 4  , "Set table temp"   , mFunc_setTableTemp);        // this menu function can be found on "LCDML_display_menuFunction" tab
// ***TIP*** Try to update _LCDML_DISP_cnt when you add a menu element.

// menu element count - last element id
// this value must be the same as the last menu element
#define _LCDML_DISP_cnt    3

// create menu
LCDML_createMenu(_LCDML_DISP_cnt);

#define B              3950 // B-коэффициент
#define SERIAL_R       102000 // сопротивление последовательного резистора, 102 кОм
#define THERMISTOR_R   100000 // номинальное сопротивления термистора, 100 кОм
#define NOMINAL_T      25 // номинальная температура (при которой TR = 100 кОм)

#define TEMP_PIN       A0
#define REALY_PIN      13
#define FUNS_PIN       10

// *********************************************************************
// SETUP
// *********************************************************************
void setup() {
  // serial init; only be needed if serial control is used
  Serial.begin(9600);                // start serial
  Serial.println(F(_LCDML_VERSION)); // only for examples

  // LCD Begin
  lcd.begin();
  lcd.backlight();

  // set special chars for scrollbar
  lcd.createChar(0, (uint8_t*)scroll_bar[0]);
  lcd.createChar(1, (uint8_t*)scroll_bar[1]);
  lcd.createChar(2, (uint8_t*)scroll_bar[2]);

  LCDML_setup(_LCDML_DISP_cnt);

  // Some settings which can be used

  // Enable Menu Rollover
  LCDML.MENU_enRollover();

  pinMode(TEMP_PIN, INPUT);
  pinMode(REALY_PIN, OUTPUT);
  digitalWrite(REALY_PIN, LOW);
}

void loop() {
  LCDML.loop();
}
