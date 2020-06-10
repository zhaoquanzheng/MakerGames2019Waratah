/*********
  Zhao Zheng
  For UNSW Maker Games 2019
  https://github.com/zhaoquanzheng/MakerGames2019Waratah
  
  Debug is only available while the Arduino IDE,Console is open 
  and chip is connected to the computer
  
  This code is built from tutorial codes and represents what I
  have learnt during this competition.
**********/
  

#ifndef WIFI
  #include <WiFi.h>
  #define WIFI
#endif
#include <DHTesp.h>

#define BUILTIN_LED 16
//#define LIGHT_SENSOR_PIN GPIO36 //not used
#define DHT_SENSOR_PIN 39 //GPIO39
#define FLOW_SENSOR_PIN 34 //GPIO34
#define RELAY_PIN 25
#define led1 26

//Copying ESP8266WifiMulti functionality
typedef struct {
  char* ssid;
  char* password;
} WiFiAPEntry;
IPAddress local_IP(192,168,1,123);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,0,0);

int DEBUG = 1;
WiFiServer server(80);
String line;
DHTesp dht;
float humidity;
float temperature;
int flowrate;
TaskHandle_t Task1;

void WiFiConnect();
void updateAllSensors();
void sendHTTPResponse(WiFiClient client);
void sendHTMLPage(WiFiClient client);

//int get_connection_status();
int get_pump_status();
int get_alerts();
float get_humidity();
float get_temperature();
int get_flowrate();
void pump_on();
void pump_off();
void pump_half();
void hello_world();
void set_led(int s);
void debug_on();
void debug_off();

String get_BUILTIN_LED_status();

void setup() {
  Serial.begin(115200);                         //For Debugging
  pinMode(BUILTIN_LED,OUTPUT);                  //For controlling LED onboard the chip
  dht.setup(DHT_SENSOR_PIN,DHTesp::DHT11);      //For setting DHT11 as our sensor
  pinMode(FLOW_SENSOR_PIN,INPUT);               //For sensing flowrate
  pinMode(RELAY_PIN,OUTPUT);                    //For controlling Relay Shield
  pinMode(led1,OUTPUT);
//  ledc_base_freq = 5000;
//  ledc_timer = 13;
  ledcSetup(0, 5000, 13);
  ledcAttachPin(RELAY_PIN, 0);
  /* Code Block for setting a static IP address
  if(!WiFi.config(local_IP,gateway,subnet)){
    Serial.println("IP config fail");
  }
  */

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  /*
  xTaskCreatePinnedToCore(
                    Task1code,   // Task function.
                    "Task1",     // name of task. 
                    10000,       // Stack size of task 
                    NULL,        // parameter of the task 
                    1,           // priority of the task 
                    &Task1,      // Task handle to keep track of created task 
                    0);          // pin task to core 0            
  */           
  //delay(500); 
  
  WiFiConnect();
  //dht.begin();
  server.begin();
  Serial.println("Setup complete");
}

void loop() {
  WiFiClient client = server.available();

  if(client){
    Serial.println("New Client");
    String currentLine = "";
    int urlLine = 1; //True
    while (client.connected()){
      if(client.available()){
        char c = client.read();
        Serial.write(c);
        line += c;
        if ( c == '\n'){
          if (urlLine == 1){
            /*
             * Serial print are debugging messages to the Computer
             * client print are return messages sent over the WiFi Connection
             */
            if (line.indexOf("GET_CONNECTION_STATUS")>0){
              Serial.println("GET_CONNECTION_STATUS"); 
              client.println("1");              //Return 1 as heartbeat message
            }else if(line.indexOf("GET_PUMP_STATUS")>0){
              Serial.println("GET_PUMP_STATUS"); 
              client.println(get_pump_status());
            }else if(line.indexOf("GET_ALERTS")>0){
              Serial.println("GET_ALERTS"); 
              client.println(get_alerts());
            }else if(line.indexOf("GET_HUMIDITY")>0){
              Serial.println("GET_HUMIDITY"); 
              client.println(get_humidity());
            }else if(line.indexOf("GET_TEMPERATURE")>0){
              Serial.println("GET_TEMPERATURE"); 
              client.println(get_temperature());
            }else if(line.indexOf("GET_FLOWRATE")>0){
              Serial.println("GET_FLOWRATE"); 
              client.println(get_flowrate());
            }else if(line.indexOf("SET_PUMP_ON")>0){
              Serial.println("SET_PUMP_ON"); 
              pump_on();
              client.println("1");
              Serial.println("pump on command received");
            }else if(line.indexOf("SET_PUMP_OFF")>0){
              Serial.println("SET_PUMP_OFF"); 
              pump_off();
              client.println("1");
            }else if(line.indexOf("SET_PUMP_HALF")>0){
              Serial.println("SET_PUMP_HALF");
              pump_half();
            }else if(line.indexOf("HELLO_WORLD")>0){
              Serial.println("HELLO_WORLD"); 
              hello_world();
            }else if(line.indexOf("SET_LED")>0){
              Serial.print("SET_LED"); 
              int index = line.indexOf("SET_LED")+7;
              int s = (int) line[index]-'0';
              set_led(s);
              Serial.println(s);
            }else if(line.indexOf("DEBUG_ON")>0){
              Serial.println("DEBUG_ON"); 
              debug_on();
            }else if(line.indexOf("DEBUG_OFF")>0){
              Serial.println("DEBUG_OFF"); 
              debug_off();
            }
          }

          if(currentLine.length()==0){          //Double \n marks end of HTTP header    
            sendHTTPResponse(client);
            sendHTMLPage(client);
          } else {                             //Single \n marks end of line
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    line = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  if(WiFi.status() != WL_CONNECTED){
    WiFiConnect();
  }
}

void WiFiConnect(){
  int numAP = 4;
  WiFiAPEntry APList[numAP]; //Change size of list manually
  APList[0] = (WiFiAPEntry) {"WiFiName","WiFiPassword"}; //Remove passwords before uploading to GitHub

  int i = 0;
  while(WiFi.status() != WL_CONNECTED){
    //try connecting to one
    WiFiAPEntry attempt = APList[i];
    i = (i+1)%numAP;
    WiFi.begin(attempt.ssid,attempt.password);
    Serial.printf("Attempting connection to %s\n",attempt.ssid);
    //wait for 5s
    delay(5000);
  }
  if (WiFi.status()==WL_CONNECTED) {
    Serial.printf("\nConnected to WiFi.\nIP address:");
    Serial.println(WiFi.localIP());
  }
}
void sendHTTPResponse(WiFiClient client){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
}
void sendHTMLPage(WiFiClient client){
  // Display the HTML web page
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // CSS to style the on/off buttons 
  // Feel free to change the background-color and font-size attributes to fit your preferences
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #555555;}</style></head>");
  //Web Page Heading
  client.println("<body><h1>Prototype Web Interface</h1>");
  //Display current stats
  client.println("<p>Builtin LED is: " + get_BUILTIN_LED_status() + "</p>");
  client.println("<p>Humidity is: " + String(get_humidity()) + "</p>");
  client.println("<p>Temperature is: " + String(get_temperature()) + "</p>");
  client.println("<p>Flowrate is: " + String(get_flowrate()) + "</p>");
  //Display some buttons
  client.println("<p><a href=\"/SET_LED0\"><button class=\"button\">LED to Off</button></a></p>");
  client.println("<p><a href=\"/SET_LED1\"><button class=\"button button2\">LED to On</button></a></p>");
  client.println("<p><a href=\"/SET_PUMP_ON\"><button class=\"button\">SET_PUMP_ON</button></a></p>");
  client.println("<p><a href=\"/SET_PUMP_OFF\"><button class=\"button button2\">SET_PUMP_OFF</button></a></p>");
  client.println("</body></html>");
  client.println();
}
int get_pump_status(){
  return 1;
}
int get_alerts(){
  return 0;
}
float get_humidity(){
  return dht.getHumidity();
}
float get_temperature(){
  return dht.getTemperature();
}
int get_flowrate(){
  return 0;
}
void pump_on(){
//  analogWrite(RELAY_PIN,HIGH);
  ledcWrite(0, 4800);
  Serial.println("RELAY PIN HIGH");
  return;
}
void pump_off(){
//  analogWrite(RELAY_PIN,LOW);
  ledcWrite(0,0);
  return;
}
void pump_half(){
//  analogWrite(RELAY_PIN,1.5);
  ledcWrite(0,3000);
  Serial.println("1.5V to RELAY_PIN");
  return;
}
void hello_world(){
  Serial.println("Hello World!");
  return;
}
void set_led(int s){
  if(s==0){
    digitalWrite(BUILTIN_LED,HIGH);
  }else{
    digitalWrite(BUILTIN_LED,LOW);
  }
  return;
}
void debug_on(){
  DEBUG = 1;
}
void debug_off(){
  DEBUG = 0;
}


String get_BUILTIN_LED_status(){
  if(digitalRead(BUILTIN_LED)==HIGH){
    return "Off";
  }else{
    return "On";
  }
}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    digitalWrite(led1, HIGH);
    delay(500);
    digitalWrite(led1, LOW);
    delay(500);
  } 
}
