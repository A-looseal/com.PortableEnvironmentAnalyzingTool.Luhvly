/**************************************************************************
  This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

  This example is for a 128x32 pixel display using I2C to communicate
  3 pins are required to interface (two I2C and one reset).

  Adafruit invests time and resources providing this open
  source code, please support Adafruit and open-source
  hardware by purchasing products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries,
  with contributions from the open source community.
  BSD license, check license.txt for more information
  All text above, and the splash screen below must be
  included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/Font5x7FixedMono.h>
#include "GravityTDS.h"

// #define ENABLE_SERIAL_OUTPUT

// OLED VARIABLES
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// OLED CSS
#define UNIT_MARQUE_DIAMETER 3

// CONDUCTIVITY VARIABLES
#define TdsSensorPin A0
GravityTDS gravityTds;

// TEMPERATURE VARIABLES
#define THERMISTOR_PROBE_PIN A1
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

// ACIDITY VARIABLES
float testValue_randomAcidity = 7.42;

// GLOBAL VARIABLES
float temperature,
    tdsValue,
    conductivity;

bool toggleConductivityMarque,
    toggleTemperatureMarque,
    toggleAcidityMarque;

void setup()
{
#ifdef ENABLE_SERIAL_OUTPUT
  Serial.begin(9600);
#endif

  pinMode(THERMISTOR_PROBE_PIN, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(100);                         // Pause for 2 seconds
  display.setFont(&Font5x7FixedMono); // choose font
  display.cp437(true);                // Use full 256 char 'Code Page 437' font
  display.clearDisplay();             // Clear the buffer

  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);      // reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024); // 1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();           // initialization
}

void loop()
{
  updateHardware(); // get an updated reading from all onboard sensors
  stateOne();       // display 'state one' Ui screen

#ifdef ENABLE_SERIAL_OUTPUT
  Serial.print(conductivity, 0);
  Serial.print(" PPM");
  Serial.print(" | ");
  Serial.print(temperature, 1);
  Serial.print("C");
  Serial.print(" | ");
  Serial.print(testValue_randomAcidity, 2);
  Serial.println("pH");
#endif

  display.display();
  delay(1000);

  toggleConductivityMarque = true;
  toggleTemperatureMarque = false;
  toggleAcidityMarque = false;

}

float getWater_Conductivity()
{
  gravityTds.setTemperature(temperature); // set the temperature and execute temperature compensation
  gravityTds.update();                    // sample and calculate
  return (gravityTds.getTdsValue());      // return calculated value
}

float getWater_Temperature()
{
  Vo = analogRead(THERMISTOR_PROBE_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  T = T - 273.15; // returns celcius
  // T = (T * 9.0) / 5.0 + 32.0; //returns farenheit

  return (T);
}

float getWater_Acidity()
{
  return (random(1.00, 9.00)); // get reading from water pH probe
}

void updateHardware()
{
  conductivity = getWater_Conductivity();       // get reading from water tds probe
  temperature = getWater_Temperature();         // get reading from water temperature probe
  testValue_randomAcidity = getWater_Acidity(); // get reading from water pH probe
}

void stateOne()
{
  // OUTPUT TO DISPLAY
  display.clearDisplay();
  marqueController();

  // draw header background
  display.fillRect(0, 0, 128, 11, SSD1306_WHITE); // draw a white rectangle
  display.setTextColor(SSD1306_BLACK);            // set text color to black
  display.setTextSize(1);                         // set text size
  display.setCursor(30, 9);                       // Start at top-left corner
  display.println("PLANT NAME");                  // print current plant/bin name to OLED

  // display settings
  display.setTextColor(SSD1306_WHITE); // set text color to black
  display.setTextSize(2);              // set text size

  // DESIRED READINGS
  display.setCursor(8, 29); // Start at top-left corner
  display.println("890");   // print desired water tds value to OLED
  display.setCursor(8, 46); // Start at top-left corner
  display.println("27.2");  // print desired water temperature value to OLED
  display.setCursor(8, 63); // Start at top-left corner
  display.println("5.55");  // print desired water pH value to OLED

  // ACTUAL READINGS
  display.setCursor(80, 29);                   // Start at top-left corner
  display.println(conductivity, 0);            // print actual water tds value to OLED
  display.setCursor(80, 46);                   // Start at top-left corner
  display.println(temperature, 1);             // print actual water temperature value to OLED
  display.setCursor(80, 63);                   // Start at top-left corner
  display.println(testValue_randomAcidity, 2); // print actual water pH value to OLED

}

void marqueController()
{
  //"TDS" or "CO2" marque
  if (toggleConductivityMarque == true) // enabled
  {
    display.fillRect(0, 17,                                      // position of the marque
                     UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER, // dimensions of the marque
                     SSD1306_WHITE);                             // tds enabled marque
  }
  else if (toggleConductivityMarque == false) // disabled
  {
    display.drawRect(0, 24,                                      // position of the marque
                     UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER, // dimensions of the marque
                     SSD1306_WHITE);                             // co2 enabled marque
  }
  // "C" or "F" marque
  if (toggleTemperatureMarque == true)
  {
    display.fillRect(0, 34,                                      // position of the marque
                     UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER, // dimensions of the marque
                     SSD1306_WHITE);                             // celcious enabled marque
  }
  else if (toggleTemperatureMarque == false)
  {
    display.drawRect(0, 41,                                      // position of the marque
                     UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER, // dimensions of the marque
                     SSD1306_WHITE);                             // farenheit enabled marque
  }
  //"pH" or "rH%" marque
  if (toggleAcidityMarque == true)
  {
    display.fillRect(0, 51,                                      // position of the marque
                     UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER, // dimensions of the marque
                     SSD1306_WHITE);                             // pH enabled marque
  }
  else if (toggleAcidityMarque == false)
  {
    display.drawRect(0, 58,                                      // position of the marque
                     UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER, // dimensions of the marque
                     SSD1306_WHITE);                             // rH% enabled marque
  }

  // arrows
  //  tds marque up/down logic
  if (conductivity > 890) // tds is too high, bring it down!
  {
    display.fillTriangle(72, 24,         // first point
                         74, 26,         // second point
                         76, 24,         // third point
                         SSD1306_WHITE); // tds down marque
  }
  else if (conductivity < 890) // tds is too low, bring it up!
  {
    display.fillTriangle(72, 19,         // first point
                         74, 17,         // second point
                         76, 19,         // third point
                         SSD1306_WHITE); // tds up marque
  }
  else if (conductivity == 890)
  {
    display.clearDisplay();
  }

  // temperature marque up/down logic
  if (temperature > 27.2) // temperature is too high, bring it down!
  {
    display.fillTriangle(72, 41,         // first point
                         74, 43,         // second point
                         76, 41,         // third point
                         SSD1306_WHITE); // temperature down marque
  }
  else if (temperature < 27.2) // temperature is too low, bring it up!
  {
    display.fillTriangle(72, 36,         // first point
                         74, 34,         // second point
                         76, 36,         // third point
                         SSD1306_WHITE); // temperature up marque
  }
  else if (temperature == 27.2)
  {
    display.clearDisplay();
  }

  // pH marque up/down logic
  if (testValue_randomAcidity > 5.55) // pH is too high, bring it down!
  {
    display.fillTriangle(72, 58,         // first point
                         74, 60,         // second point
                         76, 58,         // third point
                         SSD1306_WHITE); // pH down marque
  }
  else if (testValue_randomAcidity < 5.55) // temperature is too low, bring it up!
  {
    display.fillTriangle(72, 53,         // first point
                         74, 51,         // second point
                         76, 53,         // third point
                         SSD1306_WHITE); // pH up marque
  }
  else if (testValue_randomAcidity == 5.55)
  {
    display.clearDisplay();
  }

}