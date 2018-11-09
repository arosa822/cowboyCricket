#include <Arduino.h>
#include <Wire.h>
//for display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//lib for gas
#include <SparkFunCCS811.h>
//lib for temp / humidity
#include <Adafruit_Si7021.h>
//gas address
#define CCS811_ADDR 0x5B
#define OLED_RESET LED_BUILTIN  //4

#include "config.h"

#include "wifi.h"

//IIC addresses
CCS811 gasSensor(CCS811_ADDR);

Adafruit_Si7021 TnHSensor = Adafruit_Si7021();

Adafruit_SSD1306 display(OLED_RESET);

//variables
int CO2;
int VOC;
float temp;
float humid; 
unsigned int sensorData[2];


void getTnH()
{
  temp = TnHSensor.readTemperature();
  humid = TnHSensor.readHumidity();

}

void getGas()
{
    //Check if data is ready with .dataAvailable
    if(gasSensor.dataAvailable()){

        //If so, have the sensor read and calculate the results
        //Get them later

        gasSensor.readAlgorithmResults();

        CO2 = gasSensor.getCO2();
        VOC = gasSensor.getTVOC();

        Serial.print("CO2[");
        //Return calculated C02 reading
        Serial.print(CO2);
        Serial.print("] tVOC[");
        //Returns calculated TVOC reading
        Serial.print(VOC);
        Serial.print("] millis[");
        //Simply the time since program start
        Serial.print(millis());
        Serial.print("]");
        Serial.println();

    }

    delay(10);
    
}

void displayData()
{
      display.clearDisplay();
    

    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0,0);
      
      //1st line of data
      display.print("Bin: ");
      display.println(NAME);

      display.print("CO2: ");
      display.print(CO2);
      display.print(" VOC: ");
      display.println(VOC);

      //2nd line of data
      display.print("T: ");
      display.print(temp);
      display.print(" H: ");
      display.println(humid);

      //display.print("Count: ");
      //display.println(counter());
    
    display.display();
}

void debugDisplay(const char* theMessage)
{
      // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(theMessage);

  display.display();

  delay(750);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.println("CCS811 initializing");

  //check the return status
  CCS811Core::status returnCode = gasSensor.begin();


  //hang on if gas sensor is not reading 
  if (returnCode != CCS811Core::SENSOR_SUCCESS)
  {
      Serial.println(".begin() returned with an error.");
      while(1); //Hang if there was a problem. 
  }
  //hang on if temp / humidity sensor is not reading 
  else if (!TnHSensor.begin())
  {
    Serial.println("Did not find Si7021 sensor");
    while(1);
  }

  //set pins
  pinMode(0,OUTPUT);

  //initialize display 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  debugDisplay("Initializing...");

  initWifiManager();

  debugDisplay("wifi connected... ");

}

void loop() 
{
getGas();
getTnH();
displayData();
    display.setTextSize(1);
    display.setTextColor(WHITE);


    
    
    if (CO2 > 1000 || VOC > 1000){
        digitalWrite(0, HIGH);

    }
    else {
        digitalWrite(0, LOW);
    }

  delay(1000);
}