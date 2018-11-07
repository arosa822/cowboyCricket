/* 
Developer: Alex Rosa
Client: Cowboy Crickets
Date: 10/31/18

*/

#include "config.h"
#include <ESP8266WiFi.h>
#include "UbidotsESPMQTT.h"
#include <SoftwareSerial.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


int HTTPPORT = 80;

WiFiClient clientUbi;


    int rly = 14;
    float temp;
    float ctemp;
    float ftemp;
    float humidity;
    float ppm;
    unsigned int sensorData[2];
    
    char buffer[500];
    String response;
    const char* ubidots = "http://things.ubidots.com/api/v1.6/variables";

    unsigned long previousMillis = 0;
    const long postInt = 1000*30; 

//Ubidots client(TOKEN);

/********************************
 * Auxiliar Functions
 *******************************/
/**
 * Gets the length of the body
 * @arg variable the body of type char
 * @return dataLen the length of the variable
 */
// for posting data
int dataLen(char* variable) {
  uint8_t dataLen = 0;
  for (int i = 0; i <= 250; i++) {
    if (variable[i] != '\0') {
      dataLen++;
    } else {
      break;
    }
  }
  return dataLen;

}


void setup()
{
  // Initialise I2C communication as MASTER
  Wire.begin();

  // Initialise serial communication, set baud rate = 9600
  Serial.begin(9600);

  WiFi.begin(SSID_NAME, SSID_PASS);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
             Serial.print(".");
        }

        WiFi.setAutoReconnect(true);
        Serial.println(F("WiFi connected"));
        Serial.println(F("IP address: "));
        Serial.println(WiFi.localIP());


    pinMode(rly,OUTPUT);
    digitalWrite(rly,LOW);
    //digitalWrite(rly,LOW);
    pinMode(12,OUTPUT);
    digitalWrite(12,LOW);

}

void loop()
{
    unsigned long currentMillis = millis();
    

    GetTempHumid();

        //feedback control for relay 
        if (humidity > 50){
            digitalWrite(12, HIGH);
        }
        else 
            if(humidity< 45){

            digitalWrite(12,LOW);
        }

    delay(500);

    GetGas();

    delay(500);

    Serial.println(ftemp);
    Serial.println(humidity);
    Serial.println(ppm);

    
    //put this into it's own time state 

    //Control from Ubidots
    response = GETRequest(LABEL_DEVICE,VARIABLE_LABEL,TOKEN);

    if (response == "1.0"){
        digitalWrite(rly, HIGH);
    }
    else if (response == "0.0"){
        digitalWrite(rly, LOW);
    }

    if (currentMillis - previousMillis >= postInt){

        postData(humidity,"humidity"); 
        postData(ftemp,"temperature");       
        postData(ppm,"gas");

        previousMillis = currentMillis;

    }


}

void GetTempHumid(){

    // Start I2C transmission
    Wire.beginTransmission(TH_Addr);
    // Send humidity measurement command, NO HOLD MASTER
    Wire.write(0xF5);
    // Stop I2C transmission
    Wire.endTransmission();
    delay(500);
    
    // Request 2 bytes of data
    Wire.requestFrom(TH_Addr, 2);

    // Read 2 bytes of data
    // humidity msb, humidity lsb 
    if(Wire.available() == 2)
    {
      sensorData[0] = Wire.read();
      sensorData[1] = Wire.read();
    }
    
    // Convert the data
    humidity  = ((sensorData[0] * 256.0) + sensorData[1]);
    humidity = ((125 * humidity) / 65536.0) - 6;

    // Start I2C transmission
    Wire.beginTransmission(TH_Addr);
    // Send temperature measurement command, NO HOLD MASTER
    Wire.write(0xF3);
    // Stop I2C transmission
    Wire.endTransmission();
    delay(500);
    
    // Request 2 bytes of data
    Wire.requestFrom(TH_Addr, 2);

    // Read 2 bytes of data
    // temp msb, temp lsb
    if(Wire.available() == 2)
    {
      sensorData[0] = Wire.read();
      sensorData[1] = Wire.read();
    }

    // Convert the data
    temp  = ((sensorData[0] * 256.0) + sensorData[1]);
    ctemp = ((175.72 * temp) / 65536.0) - 46.85;
    ftemp = ctemp * 1.8 + 32;

}

void GetGas(){
      // Start I2C Transmission
    Wire.beginTransmission(GAS_Addr);
    // Select data register
    Wire.write(0x00);
    // Stop I2C transmission
    Wire.endTransmission();

    // Request 2 bytes of data
    Wire.requestFrom(GAS_Addr, 2);

    // Read 2 bytes of data
    // raw_adc msb, raw_adc lsb
    if (Wire.available() == 2)
    {
        sensorData[0] = Wire.read();
        sensorData[1] = Wire.read();
    }

    // Convert the data to 12-bits
    int raw_adc = ((sensorData[0] & 0x0F) * 256) + sensorData[1];
    ppm = (1.99 * raw_adc) / 4095.0 + 0.01;


}

void postData(float theData, const char* theVARIABLE_LABEL){

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

void parseJson(String thePayload){
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(thePayload);

    // Test if parsing succeeds.
    if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
    }

    const char* sensor = root["results"];

Serial.println(sensor);
}