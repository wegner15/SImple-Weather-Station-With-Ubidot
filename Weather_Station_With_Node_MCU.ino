/****************************************
* Include Libraries
****************************************/

#include "Ubidots.h"
#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#include <SFE_BMP180.h>
#include <Wire.h>


/****************************************
* Define Instances and Constants
****************************************/   
 
const char* UBIDOTS_TOKEN = "BBFF-GH7wFrp9hSNQ0bMaiwAGTUebM3deal";  // Put here your Ubidots TOKEN
const char* WIFI_SSID = "NovaTech";      // Put here your Wi-Fi SSID
const char* WIFI_PASS = "1234meme";      // Put here your Wi-Fi password 
int sensorPin = A0;    // input for rain sensor
int sensorValue = 0;  // variable to store the value coming from sensor Rain sensor
 
// You will need to create an SFE_BMP180 object, here called "pressure":
 
SFE_BMP180 pressure;
 #define DHTTYPE DHT11   // DHT 11
#define ALTITUDE 1795 //altitude of Nairobi in Meters 
 
#define dht_dpin 0
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);


DHT dht(dht_dpin, DHTTYPE); 

void setup() {                       

  Serial.begin(9600);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  // ubidots.setDebug(true);  // Uncomment this line for printing debug  messages  
  dht.begin();
  Serial.println("Weather Monitor\n\n");
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
 
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  delay(700);                   
}

float * BMP180_Readings(){
  float readings[4];
  char status;
  double T,P,p0,a;
 
  // Loop here getting pressure readings every 10 seconds.
 
  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  
//  Serial.print(ALTITUDE*3.28084,0);
//  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.
 
  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
 
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
 
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.
 
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
//      Serial.print("temperature: ");
//      Serial.print(T,2);
//      Serial.print(" deg C, ");
      readings[0]=T;
      
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
 
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
 
        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.
 
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
//          Serial.print("absolute pressure: ");
//          Serial.print(P,2);
//          Serial.print(" mb, ");
          readings[1]=P;
          
 
          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb
 
          p0 = pressure.sealevel(P,ALTITUDE); 
//          Serial.print("relative (sea-level) pressure: ");
//          Serial.print(p0,2);
//          Serial.print(" mb, ");
          readings[3]=p0;
          
 
          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.
 
          a = pressure.altitude(P,p0);
//          Serial.print("computed altitude: ");
//          Serial.print(a,0);
//          Serial.print(" meters, ");
          readings[3]=a;
          
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  
 
  delay(500);  // Pause for 5 seconds.
  return readings;
}


int send_to_ubidot(float Temperature=0, float Humidity=0,float Dewpoint=0, float Altitude=0, float Pressure=0, float Rain=0){
  ubidots.add("Temperature(c)", Temperature); 
  ubidots.add("Humidity(%)", Humidity);
  ubidots.add("Altitude(m)", Altitude);
  ubidots.add("Pressure(mb)", Pressure);
  ubidots.add("Dew Point", Dewpoint);
  ubidots.add("Rain", Rain);
  
  bool bufferSent = false;
  bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

  if (bufferSent) {
  // Do something if values were sent properly
   Serial.println("Values sent by the device");
   return 1;
  }
  else{
    return 0;
  }
}

void loop() {
  float Humidity = dht.readHumidity();
    float Temperature = dht.readTemperature();    
    //float F = dht.readTemperature(true);
    //float HiF = dht.computeHeatIndex(F, Humidity); //  read and compute heat index in Fahrenheit (the default)
  float Dew_point = (Temperature - (100 - Humidity) / 5);   //  dewpoint calculation using Celsius value
  //float DP = (Dew_point * 9 / 5) + 32;     //  converts dewPoint calculation to fahrenheit     
//    Serial.print("Current humidity = ");
//    Serial.print(Humidity);
//    Serial.print("%  ");
//    Serial.print("temperature = ");
//    Serial.print(Temperature); 
//    Serial.println("C  ");
//send_to_ubidot(float Temperature=0, float Humidity=0,float Dewpoint=0, float Altitude=0, float Pressure=0, float Rain=0)
float *BMP_Readings=BMP180_Readings();
float BMPTemperature=BMP_Readings[0];
float BMP_Absolute_Presure=BMP_Readings[1];
//float BMP_Relative_Pressure=BMP_Readings[2];
float BMP_Altitute=BMP_Readings[3];
int Rain=0;

sensorValue = analogRead(sensorPin);
Serial.println(sensorValue);


if (sensorValue< 700)
{
  Serial.print("rain is detected");
  Rain=1;
  }
  else
  
{
  Serial.print("rain not detected");
  Rain=0;
  }

  send_to_ubidot(Temperature,Humidity,Dew_point, BMP_Altitute, BMP_Absolute_Presure, Rain);
  delay(2000);
}
