// global defines
#define encoder_A_pin       2    // physical pin has to be 2 or 3 to use interrupts (on mega e.g. 20 or 21), use internal pullups
#define encoder_B_pin       3    // physical pin has to be 2 or 3 to use interrupts (on mega e.g. 20 or 21), use internal pullups
#define encoder_button_pin  4    // physical pin , use internal pullup

#define g_LCDML_CONTROL_button_long_press    800   // ms
#define g_LCDML_CONTROL_button_short_press   120   // ms

#define ENCODER_OPTIMIZE_INTERRUPTS //Only when using pin2/3 (or 20/21 on mega)
#include <Encoder.h> //for Encoder    Download:  https://github.com/PaulStoffregen/Encoder

Encoder ENCODER(encoder_A_pin, encoder_B_pin);

long  g_LCDML_CONTROL_button_press_time = 0;
bool  g_LCDML_CONTROL_button_prev       = HIGH;

// *********************************************************************
void lcdml_menu_control(void)
// *********************************************************************
{
  // If something must init, put in in the setup condition
  if (LCDML.BT_setup())
  {
    // runs only once

    // init pins
    pinMode(encoder_A_pin      , INPUT_PULLUP);
    pinMode(encoder_B_pin      , INPUT_PULLUP);
    pinMode(encoder_button_pin  , INPUT_PULLUP);
  }

  //Volatile Variable
  long g_LCDML_CONTROL_Encoder_position = ENCODER.read();
  bool button                           = digitalRead(encoder_button_pin);

  if (g_LCDML_CONTROL_Encoder_position <= -3) {

    if (!button)
    {
      LCDML.BT_left();
      g_LCDML_CONTROL_button_prev = LOW;
      g_LCDML_CONTROL_button_press_time = -1;
    }
    else
    {
      LCDML.BT_down();
    }
    ENCODER.write(g_LCDML_CONTROL_Encoder_position + 4);
  }
  else if (g_LCDML_CONTROL_Encoder_position >= 3)
  {

    if (!button)
    {
      LCDML.BT_right();
      g_LCDML_CONTROL_button_prev = LOW;
      g_LCDML_CONTROL_button_press_time = -1;
    }
    else
    {
      LCDML.BT_up();
    }
    ENCODER.write(g_LCDML_CONTROL_Encoder_position - 4);
  }
  else
  {
    if (!button && g_LCDML_CONTROL_button_prev) //falling edge, button pressed
    {
      g_LCDML_CONTROL_button_prev = LOW;
      g_LCDML_CONTROL_button_press_time = millis();
    }
    else if (button && !g_LCDML_CONTROL_button_prev) //rising edge, button not active
    {
      g_LCDML_CONTROL_button_prev = HIGH;

      if (g_LCDML_CONTROL_button_press_time < 0)
      {
        g_LCDML_CONTROL_button_press_time = millis();
        //Reset for left right action
      }
      else if ((millis() - g_LCDML_CONTROL_button_press_time) >= g_LCDML_CONTROL_button_long_press)
      {
        LCDML.BT_quit();
      }
      else if ((millis() - g_LCDML_CONTROL_button_press_time) >= g_LCDML_CONTROL_button_short_press)
      {
        LCDML.BT_enter();
      }
    }
  }
}
