#include <Arduino.h>
//#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//lib for gas
#include "SparkFunCCS811.h"

//lib for temp / humidity
#include "Adafruit_Si7021.h"

//gas address
#define CCS811_ADDR 0x5B

#define OLED_RESET LED_BUILTIN  //4

CCS811 gasSensor(CCS811_ADDR);

Adafruit_SSD1306 display(OLED_RESET);

//variables
int CO2;
int VOC;
unsigned int sensorData[2];

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

  //set pins
  pinMode(0,OUTPUT);

  //initialize display 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Initializing... ");
  //display.println("BoomTown!");
  display.display();

}

void loop() 
{
getGas();
    display.clearDisplay();
    

    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0,0);
      
      display.print("CO2: ");
        display.print(CO2);
            display.print(" VOC: ");
                display.println(VOC);
    
    display.display();
    
    if (CO2 > 1000 || VOC > 1000){
        digitalWrite(0, HIGH);

    }
    else {
        digitalWrite(0, LOW);
    }

  delay(1000);
}

