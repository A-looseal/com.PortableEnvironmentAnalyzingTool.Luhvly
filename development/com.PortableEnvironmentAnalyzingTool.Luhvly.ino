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

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define UNIT_MARQUE_DIAMETER 3

#define TdsSensorPin A0
GravityTDS gravityTds;

#define THERMISTOR_PROBE_PIN A1
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

float temperature,
    tdsValue;

void setup()
{
  Serial.begin(9600);

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
  delay(200);                         // Pause for 2 seconds
  display.setFont(&Font5x7FixedMono); // choose font
  display.cp437(true);                // Use full 256 char 'Code Page 437' font
  // Clear the buffer
  display.clearDisplay();

  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);      // reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024); // 1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();           // initialization
}

void loop()
{

  temperature = getWaterTemperature(THERMISTOR_PROBE_PIN); // read water temperature probe
  gravityTds.setTemperature(temperature);                  // set the temperature and execute temperature compensation
  gravityTds.update();                                     // sample and calculate
  tdsValue = gravityTds.getTdsValue();                     // then get the value

  // OUTPUT TO DISPLAY
  display.clearDisplay();

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
  display.println("585");   // print desired water tds value to OLED

  display.setCursor(8, 46); // Start at top-left corner
  display.println("58.5");  // print desired water temperature value to OLED

  display.setCursor(8, 63); // Start at top-left corner
  display.println("5.55");  // print desired water pH value to OLED

  // ACTUAL READINGS
  display.setCursor(80, 29);       // Start at top-left corner
  display.println(tdsValue, 0);    // print actual water tds value to OLED
  display.setCursor(80, 46);       // Start at top-left corner
  display.println(temperature, 1); // print actual water temperature value to OLED
  display.setCursor(80, 63);       // Start at top-left corner
  display.println("7.42");         // print actual water pH value to OLED

  // UNIT SELECTION MARQUE
  display.fillRect(0, 17,
                   UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER,
                   SSD1306_WHITE); // tds enabled marque
  display.drawRect(0, 24,
                   UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER,
                   SSD1306_WHITE); // co2 enabled marque

  display.fillRect(0, 34,
                   UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER,
                   SSD1306_WHITE); // celcious enabled marque
  display.drawRect(0, 41,
                   UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER,
                   SSD1306_WHITE); // farenheit enabled marque

  display.fillRect(0, 51,
                   UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER,
                   SSD1306_WHITE); // pH enabled marque
  display.drawRect(0, 58,
                   UNIT_MARQUE_DIAMETER, UNIT_MARQUE_DIAMETER,
                   SSD1306_WHITE); // Rh% enabled marque


//tds marque up/down logic
  if (tdsValue > 585)
  {
    display.fillTriangle(72, 19,
                         74, 17,
                         76, 19,
                         SSD1306_WHITE); // tds up marque
  }
  else if (tdsValue < 585)
  {
    display.fillTriangle(72, 24,
                         74, 26,
                         76, 24,
                         SSD1306_WHITE); // tds down marque
  }
  else if (tdsValue == 585)
  {
    display.clearDisplay();
  }
  
   
  
    

  // REQUIRED
  display.fillTriangle(72, 19, 74, 17, 76, 19, SSD1306_WHITE); // tds up marque
  display.fillTriangle(72, 24, 74, 26, 76, 24, SSD1306_WHITE); // tds down marque

  display.fillTriangle(72, 36, 74, 34, 76, 36, SSD1306_WHITE); // temperature up marque
  display.fillTriangle(72, 41, 74, 43, 76, 41, SSD1306_WHITE); // temperature down marque

  display.fillTriangle(72, 53, 74, 51, 76, 53, SSD1306_WHITE); // Ph up marque
  display.fillTriangle(72, 58, 74, 60, 76, 58, SSD1306_WHITE); // Ph down marque

  // output everything to display
  display.display();

  Serial.print(tdsValue, 0);
  Serial.print(" PPM");
  Serial.print(" | ");
  Serial.print(temperature, 0);
  Serial.println("C");
  delay(1000);
}

float getWaterTemperature(int _pin)
{
  Vo = analogRead(_pin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  T = T - 273.15; // returns celcius
  // T = (T * 9.0) / 5.0 + 32.0; //returns farenheit

  return (T);
}
