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


//includes
#include "config.h"
#include "wifi.h"

//IOT integration
#include "UbidotsESPMQTT.h"
#include <SoftwareSerial.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

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

//feedback
int controlPin = 14;

//get status of I2C bus for gas
const char* gasStatus; 

//for ubidots
char buffer[500];
String response;
const char* ubidots = "http://things.ubidots.com/api/v1.6/variables";

int HTTPPORT = 80;

WiFiClient clientUbi;

//Keep time for posting to platform
unsigned long previousMillis = 0;

//interval for posting to ubidots
const long postInt = 1000*30; 

//interval for reading values temp/humid / CO2 / VOC
const long readInt = 5000;

// for posting data
int dataLen(char* variable) 
{
  uint8_t dataLen = 0;
  for (int i = 0; i <= 250; i++) 
  {
    if (variable[i] != '\0') 
    {
      dataLen++;
    } else 
    {
      break;
    }
  }
  return dataLen;
}

void getTnH()
{
  Serial.println("Getting Temp/Humid...");
  temp = ((TnHSensor.readTemperature()*9/5)+32);
  humid = TnHSensor.readHumidity();
 

}

void getGas()
{
    Serial.println("Getting Gas...");
    //Check if data is ready with .dataAvailable
    if(gasSensor.dataAvailable()){

        //If so, have the sensor read and calculate the results
        //Get them later

        gasSensor.readAlgorithmResults();

        CO2 = gasSensor.getCO2();
        VOC = gasSensor.getTVOC();


    }

    delay(10);
    
}

void getGasStatus()
{
  CCS811Core::status returnCode = gasSensor.beginCore();
  gasStatus = "beginCore exited with: ";
  switch ( returnCode )
  {
  case CCS811Core::SENSOR_SUCCESS:
    gasStatus = "SUCCESS";
    break;
  case CCS811Core::SENSOR_ID_ERROR:
    gasStatus = "ID_ERROR";
    break;
  case CCS811Core::SENSOR_I2C_ERROR:
    gasStatus = "I2C_ERROR";
    break;
  case CCS811Core::SENSOR_INTERNAL_ERROR:
    gasStatus = "INTERNAL_ERROR";
    break;
  case CCS811Core::SENSOR_GENERIC_ERROR:
    gasStatus = "GENERIC_ERROR";
    break;
  default:
    gasStatus = "Unspecified error." ;
  }
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

      display.print(gasStatus);
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

void postData(float theData, const char* theVARIABLE_LABEL)
{

    char* body = (char *) malloc(sizeof(char) * 150);
    char* data = (char *) malloc(sizeof(char) * 300);
    /* Space to store values to send */
    char str_val[10];

    /* Read the sensor from the analog input*/
    float sensor_value = theData;

    /*---- Transforms the values of the sensors to char type -----*/
    /* 4 is mininum width, 2 is precision; float value is copied onto str_val*/
    dtostrf(sensor_value, 4, 2, str_val);

    /* Builds the body to be send into the request*/ 
    sprintf(body, "{\"%s\":%s}", theVARIABLE_LABEL, str_val);

    /* Builds the HTTP request to be POST */
    sprintf(data, "POST /api/v1.6/devices/%s", DEVICE_LABEL);
    sprintf(data, "%s HTTP/1.1\r\n", data);
    sprintf(data, "%sHost: things.ubidots.com\r\n", data);
    sprintf(data, "%sUser-Agent: %s/%s\r\n", data, USER_AGENT, VERSION);
    sprintf(data, "%sX-Auth-Token: %s\r\n", data, TOKEN);
    sprintf(data, "%sConnection: close\r\n", data);
    sprintf(data, "%sContent-Type: application/json\r\n", data);
    sprintf(data, "%sContent-Length: %d\r\n\r\n", data, dataLen(body));
    sprintf(data, "%s%s\r\n\r\n", data, body);

    /* Initial connection */
    clientUbi.connect(HTTPSERVER, HTTPPORT);

    /* Verify the client connection */
    if (clientUbi.connect(HTTPSERVER, HTTPPORT)) {
        Serial.println(F("Posting your variables: "));
        Serial.println(data);
        /* Send the HTTP Request */
        clientUbi.print(data);
    }

    /* While the client is available read the response of the server */
    while (clientUbi.available()) {
        char c = clientUbi.read();
        Serial.write(c);
    }
    /* Free memory */
    free(data);
    free(body);
    /* Stop the client */
    clientUbi.stop();
    /* Five second delay */
    delay(5000);
}

String GETRequest(const char* theLABEL, 
                  const char* theVARIABLE, 
                  const char* theTOKEN){

    //Create object of class HTTPClient
    HTTPClient http;

    int n;

    n = sprintf(buffer,"http://things.ubidots.com/api/v1.6/devices/%s/%s/lv?token=%s",theLABEL,theVARIABLE,theTOKEN);

    Serial.println(buffer);

    //call the 'begin' method and pass through the URL
    http.begin(buffer);
    int httpCode = http.GET(); 

    String thePayload = http.getString();

    Serial.println(thePayload);

    

    Serial.println("");

    //test for conditions 
    /*
    if (thePayload == "0.0"){
        Serial.println("turning off device!");
    }
    else if (thePayload == "1.0"){
        Serial.println("turning on the device!");
    }
    */

    http.end();

    return thePayload;

}

void setup() 
{
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
  pinMode(controlPin,OUTPUT);
  digitalWrite(controlPin,LOW);

  //initialize display 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  debugDisplay("Initializing...");

  initWifiManager();

  debugDisplay("wifi connected... ");
    //kick up a watchdog

}



void loop() 
{
  unsigned long currentMillis = millis();

  //read and store data from sensors
  if (currentMillis - previousMillis >= readInt)
  {
    getTnH();
    //temperature compensation for gas sensor
    delay(100);
    CCS811Core::status setEnvironmentalData(float humid, float temp );
    getGas();
    displayData();
    getGasStatus();
    
    previousMillis = currentMillis;
  }

  //post and get data from Unibidots

  if (currentMillis - previousMillis >= postInt)
  {
    postData(humid,"Humidity");
    postData(temp,"Temperature");
    postData(CO2,"CO2");
    postData(VOC, "VOC");
    response = GETRequest(LABEL_DEVICE, VARIABLE_LABEL,TOKEN);

    previousMillis = currentMillis;
  }

  if (response == "1.0")
  {
    digitalWrite(controlPin,HIGH);
  }
  else if (response == "0.0")
  {
    digitalWrite(controlPin,LOW);
  }

}