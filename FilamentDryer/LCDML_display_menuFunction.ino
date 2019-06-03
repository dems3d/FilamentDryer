#include <TimeLib.h>
#include <EEPROM.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS      7

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#define EEPROM_TEMP         0
#define EEPROM_TIME_H       1
#define EEPROM_TIME_M       2
#define EEPROM_TABLE_TEMP   3

uint8_t eeprom_temperature;
uint8_t eeprom_minutes;
uint8_t eeprom_hours;
uint8_t eeprom_table_temperature;
bool    hours_minutes;

int S = 0;
int M = 0;
int H = 0;
bool time_go;
bool temp_box_go;
bool temp_table_go;

void mFunc_start(uint8_t param) {
  if (LCDML.FUNC_setup()) {
    analogWrite(FUNS_PIN, 0);

    lcd.noBlink();

    DS18B20.begin();

    eeprom_temperature = EEPROM.read(EEPROM_TEMP);
    if (eeprom_temperature == 255) {
      eeprom_temperature = 40;
      EEPROM.write(EEPROM_TEMP, eeprom_temperature);
    }

    eeprom_table_temperature = EEPROM.read(EEPROM_TABLE_TEMP);
    if (eeprom_table_temperature == 255) {
      eeprom_table_temperature = 60;
      EEPROM.write(EEPROM_TABLE_TEMP, eeprom_table_temperature);
    }

    eeprom_hours = EEPROM.read(EEPROM_TIME_H);
    if (eeprom_hours == 255) {
      eeprom_hours = 3;
      EEPROM.write(EEPROM_TIME_H, eeprom_hours);
    }

    eeprom_minutes = EEPROM.read(EEPROM_TIME_M);
    if (eeprom_minutes == 255) {
      eeprom_minutes = 0;
      EEPROM.write(EEPROM_TIME_M, eeprom_minutes);
    }

    lcd.setCursor(0, 0);
    lcd.print("TEMP:");
    lcd.setCursor(9, 0);
    lcd.print("/");
    lcd.setCursor(11, 0);
    lcd.print(String(eeprom_temperature));

    lcd.setCursor(2, 1);
    lcd.print(":");
    lcd.setCursor(5, 1);
    lcd.print(":");

    lcd.setCursor(10, 1);
    lcd.print("Tb:");

    LCDML.FUNC_setLoopInterval(1000);

    S = 0;
    M = eeprom_minutes;
    H = eeprom_hours;
    time_go = true;
    temp_box_go = true;
    temp_table_go = true;
  }

  if (LCDML.FUNC_loop()) {
    DS18B20.requestTemperatures();
    float box_temperature = DS18B20.getTempCByIndex(0);

    lcd.setCursor(6, 0);
    lcd.print(box_temperature, 0);

    int t = analogRead(TEMP_PIN);
    float tr = 1023.0 / t - 1;
    tr = SERIAL_R / tr;

    float table_temperature;
    table_temperature = tr / THERMISTOR_R; // (R/Ro)
    table_temperature = log(table_temperature); // ln(R/Ro)
    table_temperature /= B; // 1/B * ln(R/Ro)
    table_temperature += 1.0 / (NOMINAL_T + 273.15); // + (1/To)
    table_temperature = 1.0 / table_temperature; // Invert
    table_temperature -= 273.15;

    lcd.setCursor(13, 1);
    lcd.print(table_temperature, 0);

    if (table_temperature < 100) {
      lcd.setCursor(15, 0);
      lcd.print(" ");
    }

    S--;
    if (S < 0) {
      M--;
      S = 59;
    }
    if (M < 0) {
      H--;
      M = 59;
    }
    if (H < 0) {
      time_go = false;

      H = 0;
      M = 0;
      S = 0;

      digitalWrite(REALY_PIN, LOW);
      analogWrite(FUNS_PIN, 255);
    }

    if (H > 9) {
      lcd.setCursor(0, 1);
      lcd.print (H);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(H);
      lcd.setCursor(2, 1);
      lcd.print(":");
    }

    if (M > 9) {
      lcd.setCursor(3, 1);
      lcd.print(M);
    } else {
      lcd.setCursor(3, 1);
      lcd.print("0");
      lcd.setCursor(4, 1);
      lcd.print(M);
      lcd.setCursor(5, 1);
      lcd.print(":");
    }

    if (S > 9) {
      lcd.setCursor(6, 1);
      lcd.print(S);
    } else {
      lcd.setCursor(6, 1);
      lcd.print("0");
      lcd.setCursor(7, 1);
      lcd.print(S);
      lcd.setCursor(8, 1);
      lcd.print(" ");
    }

    if (time_go) {
      if (temp_box_go) {
        if (box_temperature <= eeprom_temperature) {
          if (temp_table_go) {
            if (table_temperature <= eeprom_table_temperature) {
              digitalWrite(REALY_PIN, HIGH);
            } else {
              temp_table_go = false;
              
              digitalWrite(REALY_PIN, LOW);
            }
          } else {
            if (table_temperature <= eeprom_table_temperature - 5) {
              temp_table_go = true;
            }
          }

          analogWrite(FUNS_PIN, 0);
        } else {
          temp_box_go = false;

          digitalWrite(REALY_PIN, LOW);

          analogWrite(FUNS_PIN, 255);
        }
      } else {
        if (box_temperature <= eeprom_temperature - 5) {
          temp_box_go = true;
        }
      }
    }

    if (LCDML.BT_checkEnter()) {
      LCDML.FUNC_goBackToMenu();
    }

    if (LCDML.BT_checkRight() || LCDML.BT_checkDown()) {
      LCDML.BT_resetRight(); // reset the left button
      LCDML.BT_resetDown(); // reset the left button

      if (eeprom_temperature < 85) {
        eeprom_temperature++;
      }

      lcd.setCursor(11, 0); // set cursor
      lcd.print(eeprom_temperature); // print change content
    }

    if (LCDML.BT_checkLeft() || LCDML.BT_checkUp()) {
      LCDML.BT_resetLeft(); // reset the left button
      LCDML.BT_resetUp(); // reset the left button

      if (eeprom_temperature > 0) {
        eeprom_temperature--;
      }

      lcd.setCursor(11, 0); // set cursor
      lcd.print(eeprom_temperature); // print change content
    }

    if (eeprom_temperature < 10) {
      lcd.setCursor(12, 0);
      lcd.print(" ");
    }
  }

  if (LCDML.FUNC_close()) {
    digitalWrite(REALY_PIN, LOW);

    analogWrite(FUNS_PIN, 0);
  }
}

void mFunc_setBoxTemp(uint8_t param) {
  if (LCDML.FUNC_setup()) {
    lcd.noBlink();

    eeprom_temperature = EEPROM.read(EEPROM_TEMP);
    if (eeprom_temperature == 255) {
      eeprom_temperature = 40;
      EEPROM.write(EEPROM_TEMP, eeprom_temperature);
    }

    lcd.setCursor(0, 0); // set cursor
    lcd.print("SET TEMPERATURE");

    lcd.setCursor(0, 1); // set cursor
    lcd.print(eeprom_temperature);

    // Disable the screensaver for this function until it is closed
    LCDML.FUNC_disableScreensaver();
  }

  if (LCDML.FUNC_loop())  {
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      if (LCDML.BT_checkRight() || LCDML.BT_checkDown()) // check if button left is pressed
      {
        LCDML.BT_resetRight(); // reset the left button
        LCDML.BT_resetDown(); // reset the left button

        if (eeprom_temperature < 120) {
          eeprom_temperature++;
        }

        lcd.setCursor(0, 1); // set cursor
        lcd.print(eeprom_temperature); // print change content
      }

      if (LCDML.BT_checkLeft() || LCDML.BT_checkUp()) // check if button left is pressed
      {
        LCDML.BT_resetLeft(); // reset the left button
        LCDML.BT_resetUp(); // reset the left button

        if (eeprom_temperature > 0) {
          eeprom_temperature--;
        }

        lcd.setCursor(0, 1); // set cursor
        lcd.print(eeprom_temperature); // print change content
      }


      if (eeprom_temperature < 100) {
        lcd.setCursor(2, 1);
        lcd.print(" ");
      }

      if (eeprom_temperature < 10) {
        lcd.setCursor(1, 1);
        lcd.print(" ");
      }
    }

    // check if button count is three
    if (LCDML.BT_checkEnter()) {
      LCDML.FUNC_goBackToMenu();      // leave this function
    }

  }

  if (LCDML.FUNC_close()) {
    EEPROM.write(EEPROM_TEMP, eeprom_temperature);
  }
}

void mFunc_setTime(uint8_t param) {
  if (LCDML.FUNC_setup()) {
    eeprom_hours = 0;
    eeprom_minutes = 0;
    hours_minutes = false;

    eeprom_hours = EEPROM.read(EEPROM_TIME_H);
    if (eeprom_hours == 255) {
      eeprom_hours = 3;
      EEPROM.write(EEPROM_TIME_H, eeprom_hours);
    }

    eeprom_minutes = EEPROM.read(EEPROM_TIME_M);
    if (eeprom_minutes == 255) {
      eeprom_minutes = 0;
      EEPROM.write(EEPROM_TIME_M, eeprom_minutes);
    }

    lcd.setCursor(0, 0); // set cursor
    lcd.print("SET TIME");
    lcd.setCursor(2, 1); // set cursor
    lcd.print(":");

    lcd.setCursor(3, 1);
    if (eeprom_minutes == 0) {
      lcd.print("00");
    } else {
      lcd.print(eeprom_minutes);
    }

    if (eeprom_hours < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1); // set cursor
    } else {
      lcd.setCursor(0, 1);
    }

    lcd.print(eeprom_hours);

    lcd.setCursor(0, 1);
    lcd.blink();

    // Disable the screensaver for this function until it is closed
    LCDML.FUNC_disableScreensaver();
  }

  if (LCDML.FUNC_loop())  {
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      if (LCDML.BT_checkRight() || LCDML.BT_checkDown()) // check if button left is pressed
      {
        LCDML.BT_resetRight(); // reset the left button
        LCDML.BT_resetDown(); // reset the left button
        if (!hours_minutes) {
          if (eeprom_hours < 24) {
            eeprom_hours++;
          }

          if (eeprom_hours < 10) {
            lcd.setCursor(1, 1); // set cursor

          } else {
            lcd.setCursor(0, 1); // set cursor
          }
          lcd.print(eeprom_hours); // print change content
        } else {
          lcd.setCursor(3, 1);
          if (eeprom_minutes == 0) {
            eeprom_minutes = 30;
            lcd.print(eeprom_minutes);
          } else {
            eeprom_minutes = 0;
            lcd.print("00");
          }
        }
      }

      if (LCDML.BT_checkLeft() || LCDML.BT_checkUp()) // check if button left is pressed
      {
        LCDML.BT_resetLeft(); // reset the left button
        LCDML.BT_resetUp(); // reset the left button
        if (!hours_minutes) {
          if (eeprom_hours > 0) {
            eeprom_hours--;
          }

          if (eeprom_hours < 10) {
            lcd.setCursor(1, 1); // set cursor
          } else {
            lcd.setCursor(0, 1); // set cursor
          }
          lcd.print(eeprom_hours); // print change content
        } else {
          lcd.setCursor(3, 1);
          if (eeprom_minutes == 0) {
            eeprom_minutes = 30;
            lcd.print(eeprom_minutes);
          } else {
            eeprom_minutes = 0;
            lcd.print("00");
          }
        }
      }

      if (hours_minutes) {
        if (eeprom_hours < 10) {
          lcd.setCursor(0, 1);
          lcd.print("0");
        }
      }

      if (!hours_minutes) {
        lcd.setCursor(0, 1);
        lcd.blink();
      } else {
        lcd.setCursor(3, 1);
        lcd.blink();
      }
    }

    // check if button count is three
    if (LCDML.BT_checkEnter()) {
      if (hours_minutes) {
        LCDML.FUNC_goBackToMenu();
      } else {
        hours_minutes = true;

        lcd.setCursor(3, 1);
        lcd.blink();
      }
    }
  }

  if (LCDML.FUNC_close()) {
    EEPROM.write(EEPROM_TIME_H, eeprom_hours);
    EEPROM.write(EEPROM_TIME_M, eeprom_minutes);
  }
}

void mFunc_setTableTemp(uint8_t param) {
  if (LCDML.FUNC_setup()) {
    lcd.noBlink();

    eeprom_table_temperature = EEPROM.read(EEPROM_TABLE_TEMP);
    if (eeprom_table_temperature == 255) {
      eeprom_table_temperature = 60;
      EEPROM.write(EEPROM_TABLE_TEMP, eeprom_table_temperature);
    }

    lcd.setCursor(0, 0); // set cursor
    lcd.print("SET TABLE TEMP");

    lcd.setCursor(0, 1); // set cursor
    lcd.print(eeprom_table_temperature);

    // Disable the screensaver for this function until it is closed
    LCDML.FUNC_disableScreensaver();
  }

  if (LCDML.FUNC_loop())  {
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      if (LCDML.BT_checkRight() || LCDML.BT_checkDown()) // check if button left is pressed
      {
        LCDML.BT_resetRight(); // reset the left button
        LCDML.BT_resetDown(); // reset the left button

        if (eeprom_table_temperature < 120) {
          eeprom_table_temperature++;
        }

        lcd.setCursor(0, 1); // set cursor
        lcd.print(eeprom_table_temperature); // print change content
      }

      if (LCDML.BT_checkLeft() || LCDML.BT_checkUp()) // check if button left is pressed
      {
        LCDML.BT_resetLeft(); // reset the left button
        LCDML.BT_resetUp(); // reset the left button

        if (eeprom_table_temperature > 0) {
          eeprom_table_temperature--;
        }

        lcd.setCursor(0, 1); // set cursor
        lcd.print(eeprom_table_temperature); // print change content
      }


      if (eeprom_table_temperature < 100) {
        lcd.setCursor(2, 1);
        lcd.print(" ");
      }

      if (eeprom_table_temperature < 10) {
        lcd.setCursor(1, 1);
        lcd.print(" ");
      }
    }

    // check if button count is three
    if (LCDML.BT_checkEnter()) {
      LCDML.FUNC_goBackToMenu();      // leave this function
    }

  }

  if (LCDML.FUNC_close()) {
    EEPROM.write(EEPROM_TABLE_TEMP, eeprom_table_temperature);
  }
}
