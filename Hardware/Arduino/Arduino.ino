#include <ArduinoJson.h>
#include <dht.h>
#include <SoftwareSerial.h>

//Instances that is used by the Arduino
dht DHT;
SoftwareSerial node(2, 3); //RX | TX

//---------------------------------------------------------------------------

//Pin declirations
#define DHT11_PIN 7
#define MOIS_PIN A0

#define PRES_PIN A1
#define RELAY_PIN 6

//---------------------------------------------------------------------------

//Variables
float temp;
float humi;
float mois;

int lightVal;
//---------------------------------------------------------------------------

void setup() {
  //Creating the serial comunications 9600 Baud Rate
  Serial.begin(9600);
  node.begin(9600);
  delay(1000);

  pinMode (RELAY_PIN, OUTPUT);

  Serial.println("Program Started");
}

void loop() {
  //Json buffer that allows 1000 bits of data to be send and received
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  
  //Temp and Humidity
  //--------------------------------------------------------------------------

  //Analog data is read from the Arduino
  float chk = DHT.read11(DHT11_PIN);
  
  //The DHT library automatically converts the data from Analog to Human readable data 
  temp = DHT.temperature;
  humi = DHT.humidity;

  //Store the data in the Json object file.
  data["temp"] = temp;
  data["humi"] = humi;
  //--------------------------------------------------------------------------

  //Moisture levels and decisions
  //# the sensor value description
  //# 0  ~300     dry soil
  //# 300~700     humid soil
  //# 700~950     in water

  //Analog data is read from the Arduino
  mois = analogRead(MOIS_PIN);
  //Store the data in the Json object file.
  data["mois"] = mois;
  
  //---------------------------------------------------------------------------
  //Light Sensor

  //Analog data is read from the Arduino
  lightVal = analogRead(PRES_PIN);
  //Store the data in the Json object file.
  data["light"] = lightVal;
  //---------------------------------------------------------------------------

  //Sending data to ESP for server transfer and printing the data for debugging purposes.
  data.printTo(node);
  data.printTo(Serial);
  Serial.println();
  jsonBuffer.clear();
  //--------------------------------------------------------------------------
  
  //Decision: The analog data is checked and according to the calibration it decides if the water 
  //pump needs to turn on or off
  if (mois <= 450)
  {
    digitalWrite(RELAY_PIN, HIGH);
  }
  else{
    digitalWrite(RELAY_PIN, LOW);
  }
  
  //--------------------------------------------------------------------------
  //Code based simulations (Hardware we could not afford)

  //Decision: The analog data is checked and according to the calibration it decides if the fans 
  //need to be turn on or off
  if (temp >= 30)
  {
    Serial.println("Turning on fans")
  }
  else{
    //if (Fans_Pin == HIGH) 
    Serial.println("Turning off fans")

  }

  //Decision: The analog data is checked and according to the calibration it decides if the lights 
  //need to be turn on or off
  if (lightVal <= 100)
  {
    Serial.println("Going Dark: Switching on lights for nightime development")
  }
  else{
    Serial.println("Sun is shining: Switching to free light power --> Sun active")
  }
  //--------------------------------------------------------------------------

  delay(5000);
}
