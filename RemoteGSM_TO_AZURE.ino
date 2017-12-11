#include <SoftwareSerial.h>

//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 8

//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 7

//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);

void setup() {
//Begin serial comunication with Arduino and Arduino IDE (Serial Monitor)
Serial.begin(9600);
while(!Serial);

//Being serial communication witj Arduino and SIM800
serialSIM800.begin(9600);
delay(1000);

setupGPRS();
initHTTPS();
Serial.println("Setup Complete!");

}

void loop() {
 //Read SIM800 output (if available) and print it in Arduino IDE Serial Monitor
  if(serialSIM800.available())
  {
    sendDataToAzure("-10");    

    Serial.write(serialSIM800.read());
  }
//Read Arduino IDE Serial Monitor inputs (if available) and send them to SIM800
  if(Serial.available())
  {
    serialSIM800.write(Serial.read());
  }
}



bool sendDataToAzure(const char* val)
{
  delay( 250 );
      // Set bearer profile identifier
      cleanBuffer();
      serialSIM800.println("AT+HTTPPARA=\"CID\",1");
      if ( waitFor("OK", "ERROR") != 1 ) return false;
  delay( 250 );
      // Set the URL
      cleanBuffer();
      serialSIM800.print("AT+HTTPPARA=\"URL\",\"https://XXXXXX.azure-devices.net/devices/YYYYYY/messages/events?api-version=2016-02-03\"");
      if ( waitFor("OK", "ERROR") != 1 ) return false;
  delay( 250 );
      //Set the shared access signature
      cleanBuffer();
      serialSIM800.print("AT+HTTPPARA=\"USERDATA\",\"Authorization: SharedAccessSignature sr=XXXXXX.azure-devices.net&sig=XYZXYZXYZXYZXYZXYZdQSiv4%3D&se=0000000000\"");
      if ( waitFor("OK", "ERROR") != 1 ) return false;
  delay( 250 );
      //Set Content-Type field in the HTTP header
      cleanBuffer();
      serialSIM800.print("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
      if ( waitFor("OK", "ERROR") != 1 ) return false;
  delay( 250 );
      //Set the JSON string to be sent:
      cleanBuffer();
      serialSIM800.print("AT+HTTPDATA=100,5000");
      serialSIM800.print("{\"sensorValue\":\"1\",\"sensorValue2\":\"3\",\"sensorValue3\":\"5\"}");
      
      if ( waitFor("OK", "ERROR") != 1 ) return false;
  delay( 250 );
      //HTTP POST
      cleanBuffer();
      serialSIM800.print("AT+HTTPACTION=1");
      if ( waitFor("OK", "ERROR") != 1 ) return false;
  
  return true;
  
}



/****************************************************************************************/
bool setupGPRS()
{
    delay( 250 );
    if ( !setConnectionType() ){
      Serial.println("Failed to set connection type to GPRS.");
      return false;
    }
    delay( 250 );
    if ( !setAPN("internet.wind") ){
      Serial.println("Failed to setup APN.");
      return false;
    }
    delay( 250 );
    if ( !startGPRS() ){
      Serial.println("Failed to start GPRS.");
      //return false;
    }
    delay( 250 );
    if ( !hasIP() ){
      Serial.println("Failed to acquire IP address.");
      return false;
    }

    return true;
}

bool setConnectionType()
{
    cleanBuffer();
    serialSIM800.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    if ( waitFor("OK", "ERROR") != 1 ) return false;

    return true;
}

bool setAPN(const char* apn)
{
    cleanBuffer();
    serialSIM800.print("AT+SAPBR=3,1,\"APN\",\"");
    serialSIM800.print(apn);
    serialSIM800.println("\"");
    if ( waitFor("OK", "ERROR") != 1 ) return false;

    return true;
}

bool startGPRS()
{
    cleanBuffer();
    serialSIM800.println("AT+SAPBR=1,1");
    if ( waitFor("OK", "ERROR") != 1 ) return false;

    return true;
}

bool hasIP()
{
    cleanBuffer();
    serialSIM800.println("AT+SAPBR=2,1");
    if ( waitFor("0.0.0.0", "OK") != 2 ) return false;

    return true;
}
/****************************************************************************************/
bool initHTTPS()
{  
  // Enable HTTP functions
  cleanBuffer();
  serialSIM800.println("AT+HTTPINIT");
  if ( waitFor("OK", "ERROR") != 1 ) return false;

  // Enable SSL
  cleanBuffer();
  serialSIM800.println("AT+HTTPSSL=1");
  if ( waitFor("OK", "ERROR") != 1 ) return false;  
}
/****************************************************************************************/




/******************************************
 * 
 *    UTILITIES
 * 
 *****************************************/
void cleanBuffer()
{
    delay( 250 );
    while ( serialSIM800.available() > 0) 
    {
        serialSIM800.read();    // Clean the input buffer
        delay(50);
    }
}
int8_t waitFor(const char* expected_answer1, const char* expected_answer2)
{
    uint8_t x=0, answer=0;
    char response[100];
    unsigned long previous;

    memset(response, (char)0, 100);    // Initialize the string

    delay( 250 );

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(serialSIM800.available() > 0){
            response[x] = serialSIM800.read();
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL)
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            else if (strstr(response, expected_answer2) != NULL)
            {
                answer = 2;
            }
        }
        delay(10);
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < 10000 ));

    return answer;
}


