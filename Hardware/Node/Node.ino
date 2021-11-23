#include<ArduinoJson.h>
#include<SoftwareSerial.h>
#include <WebSocketClient.h>
#include <ESP8266WiFi.h>
//--------------------------------------------

//Variables declirations
String passData;
float temp;
float humi;
float mois;
int lightVal;
boolean handshakeFailed=0;

//This is the configuration for the Node device and with some 
//sensitive data that is needed for it to connect to the internet and server
char path[] = "/NodeOne";
const char* ssid     = "Delport-WiFi 2.4GHz";
const char* password = "Adp!001G";

char* host = "51.195.104.141";
const int espport= 6579;

//--------------------------------------------

//Basic Instances
WebSocketClient webSocketClient;
//SRX=DPin-D2; STX-DPin-D1
SoftwareSerial node(D2, D1);
WiFiClient client;

//--------------------------------------------
void setup()
{
  //enable Serial Monitor
  Serial.begin(9600);
  //enable Node Port
  node.begin(9600); 
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  //Initialize the wifi protocol according to configurations above.
  WiFi.begin(ssid, password);
  
  //This function will run until a wifi connection is made.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  //Displays the status and IP address of the network the device connected to.
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
  
  //Calls the WebSocket connect function to initialize a handshake with the server.
  wsconnect();
  
  while(!Serial) continue;
}
  
void loop()
{
  //Receiving data from Arduino inside a exact replica Json object and buffer 
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(node);
  
  //Decision: This checks if the data received is correct of corrupt and the either 
  //clears the buffer or continues.
  if (data == JsonObject::invalid())
  {
    jsonBuffer.clear();
    return;
  }
  
  //Fill variables with data from Json Object.
  temp = data["temp"];
  humi = data["humi"];
  mois = data["mois"];
  lightVal = data["light"];
  
  //--------------------------------------------

  //Posting data to server
  if (client.connected()) {    
    //Create a single line string seperated with comas to send in a format to backend server.  
    passData = (String)temp + "," + (String)humi + "," + (String)mois + "," + (String)lightVal;
    //send sensor data to websocket server
    webSocketClient.sendData(passData);
  }
  //--------------------------------------------
}

void wsconnect(){
  // Connect to the websocket server
  if (client.connect(host, espport)) {
    Serial.println("Connected");
  } 
  else {
    //If connection is refused or lost this function will constantly check until it connects.
    Serial.println("Connection failed.");
    delay(1000);  
   
    if(handshakeFailed){
      handshakeFailed=0;
      ESP.restart();
    }
    handshakeFailed=1;
  }
  
  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  
  //We have connected successfully
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } 
  else {
    Serial.println("Handshake failed.");
    delay(4000);  
   
    //After a fail we reset the entire device and try again until successful
    if(handshakeFailed){
      handshakeFailed=0;
      ESP.restart();
    }
    handshakeFailed=1;
  }
}
//--------------------------------------------
