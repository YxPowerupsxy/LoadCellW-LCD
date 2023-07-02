#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif 

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
const uint8_t I2C_ADDRESS = 0x27;
const uint8_t LCD_CHAR = 16;
const uint8_t LCD_ROW = 2;
LiquidCrystal_I2C lcd(0x27, I2C_ADDRESS, LCD_CHAR, LCD_ROW);

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;


void setup()
{
  Wire.begin();
  lcd.begin();
  lcd.backlight();

  lcd.clear();
  lcd.print("Stand in Weight");
  lcd.setCursor(0, 1);
  lcd.print("Lets Start");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("Press the button");


   float calibrationValue; // calibration value
  calibrationValue = 696.0; // uncomment this if you want to set this value in the sketch
#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch this value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch this value from eeprom

  LoadCell.begin();
  //LoadCell.setReverseOutput();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  }
  else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }
}

void loop() {

if (button.getSingleDebouncedRelease())
{ 
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      lcd.clear();
      lcd.print("Please Weight");
      lcd.setCursor(0, 1);
      lcd.print("Load: ");
      Serial.println(i);
      float Kg = i / 35.273962;
      clearCharacters(1, 9, LCD_CHAR - 1 );
      lcd.setCursor (9, 1); //
      lcd.print(Kg);
      lcd.print("Kg");
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
    Serial.print("Tare complete");
  }
}
}
void clearCharacters(uint8_t row, uint8_t start, uint8_t stop )
{
  for (int i = start; i <= stop; i++)
  {
    lcd.setCursor (i, row); //
    lcd.write(254);
  }
}
