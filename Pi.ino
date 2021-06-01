// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include "ESPAsyncWebServer.h"
#include <HTU21D.h>

//-----------------------------------network credentials
const char* ssid = "***********";
const char* password = "**********";

//------------------------------------web Server HTTP Authentication credentials
const char* http_username = "admin";
const char* http_password = "admin";
//----------------------------------------Host & httpsPort google spreadsheet
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbzEzSk_6quzZoZL17J3OTXkrjdI2EpBn7S8fiwNt-NiMZsh-IvE"; //--> spreadsheet script ID
void sendData(float tem, int hum);
WiFiClientSecure client;
//---------------------------------------- 

HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14); // HTU21 connect to ESP8266 I2C
const int output = 32;       // Output socket
const int motionSensor = 13; // motion Sensor
int ledState = LOW;           // current state of the output led 
bool motionDetected = false;  // flag variable to send motion alert message
bool clearMotionAlert = true; // clear last motion alert message 
unsigned long startMillis=millis(); // time from start to send data to spreadsheet

//-----------------------------Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

const char* PARAM_INPUT_1 = "state";

//-------------------------Check if motion was detected
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
  clearMotionAlert = false;
}

// Main HTML web page in root url /
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h3 {font-size: 1.8rem; color: white;}
    h4 { font-size: 1.2rem;}
    p { font-size: 1.4rem;}
    body {  margin: 0;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px; margin-bottom: 20px;}
    .switch input {display: none;}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 68px;   opacity: 0.8;   cursor: pointer;}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #1b78e2}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
    .topnav { overflow: hidden; background-color: #1b78e2;}
    .content { padding: 20px;}
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);}
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}
    .slider2 { -webkit-appearance: none; margin: 14px;  height: 20px; background: #ccc; outline: none; opacity: 0.8; -webkit-transition: .2s; transition: opacity .2s; margin-bottom: 40px; }
    .slider:hover, .slider2:hover { opacity: 1; }
    .slider2::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 40px; height: 40px; background: #008B74; cursor: pointer; }
    .slider2::-moz-range-thumb { width: 40px; height: 40px; background: #008B74; cursor: pointer;}
    .reading { font-size: 2.6rem;}
    .card-switch {color: #50a2ff; }
    .card-HTU{ color: #572dfb;}
    .card-motion{ color: #3b3b3b; cursor: pointer;}
    .icon-pointer{ cursor: pointer;}
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ESP <span style="text-align:right;">&nbsp;&nbsp; <i class="fas fa-user-slash icon-pointer" onclick="logoutButton()"></i></span></h3>
  </div>
  <div class="content">
    <div class="cards">
      %BUTTONPLACEHOLDER%
      <div class="card card-HTU">
        <h4><i class="fas fa-chart-bar"></i> TEMPERATURE</h4><div><p class="reading"><span id="temp"></span>&deg;C</p></div>
      </div>
      <div class="card card-HTU">
        <h4><i class="fas fa-chart-bar"></i> HUMIDITY</h4><div><p class="reading"><span id="humi"></span>&percnt;</p></div>
      </div>
      <div class="card card-motion" onClick="clearMotionAlert()">
        <h4><i class="fas fa-running"></i> MOTION SENSOR</h4><div><p class="reading"><span id="motion">%MOTIONMESSAGE%</span></p></div>
      </div>
  </div>
<script>
function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}

function toggleLed(element) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/toggle", true);
  xhr.send();
}
function clearMotionAlert() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/clear-motion", true);
  xhr.send();
  setTimeout(function(){
    document.getElementById("motion").innerHTML = "No motion";
    document.getElementById("motion").style.color = "#3b3b3b";
  }, 1000);
}
if (!!window.EventSource) {
 var source = new EventSource('/events');
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 source.addEventListener('led_state', function(e) {
  console.log("led_state", e.data);
  var inputChecked;
  if( e.data == 1){ inputChecked = true; }
  else { inputChecked = false; }
  document.getElementById("led").checked = inputChecked;
 }, false);
 source.addEventListener('motion', function(e) {
  console.log("motion", e.data);
  document.getElementById("motion").innerHTML = e.data;
  document.getElementById("motion").style.color = "#b30000";
 }, false); 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("humi").innerHTML = e.data;
 }, false);
}</script>
</body>
</html>)rawliteral";

String outputState(int gpio){
  if(digitalRead(gpio)){
    return "checked";
  }
  else {
    return "";
  }
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons;
    String outputStateValue = outputState(19);
    buttons+="<div class=\"card card-switch\"><h4><i class=\"fas fa-lightbulb\"></i> STATUS LED</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleLed(this)\" id=\"led\" " + outputStateValue + "><span class=\"slider\"></span></label></div>";
    return buttons;
  }
  else if(var == "MOTIONMESSAGE"){
    if(!clearMotionAlert) {
      return String("<span style=\"color:#b30000;\">MOTION DETECTED!</span>");
    }
    else {
      return String("No motion");
    }
  }
  return String();
}

// Logged out web page
const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

void setup(){
  // Serial port for debugging
  Serial.begin(115200);
    
  if (!myHTU21D.begin()) {
    Serial.println("Could not find a valid HTU21D sensor!");
    while (1);
  }
  
  // initialize the LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !ledState); // default ON
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode to trigger when the pin goes from low to high
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  //--------------Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
  });
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });
  // Send a GET request to control output socket <ESP_IP>/output?state=<inputMessage>
  server.on("/output", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    String inputMessage;
    // GET gpio and state value
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      digitalWrite(output, inputMessage.toInt());
      request->send(200, "text/plain", "OK");
    }
    request->send(200, "text/plain", "Failed");
  });
  // Send a GET request to control on board status LED <ESP_IP>/toggle
  server.on("/toggle", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, !ledState);
    request->send(200, "text/plain", "OK");
  });
  // Send a GET request to clear the "Motion Detected" message <ESP_IP>/clear-motion
  server.on("/clear-motion", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    clearMotionAlert = true;
    request->send(200, "text/plain", "OK");
  });
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis and set reconnect delay to 1 second
    client->send("hello!",NULL,millis(),1000);
  });
  server.addHandler(&events);
  
  client.setInsecure();
  
  // Start server
  server.begin();
  
  startMillis=millis();
}
 
void loop(){
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 10000;
  int after_send_data=300000;
   if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    float temp=myHTU21D.readTemperature();
    float hum=myHTU21D.readHumidity();
    events.send(String(temp).c_str(),"temperature",millis());
    events.send(String(hum).c_str(),"humidity",millis());
    unsigned long currentMillis = millis();
    if(currentMillis - startMillis>=after_send_data)
    {
    sendData(temp,hum);
    startMillis = currentMillis;
    }
    lastEventTime = millis();
  }

  if(motionDetected & !clearMotionAlert){
    events.send(String("MOTION DETECTED!").c_str(),"motion",millis());
    motionDetected = false;
  } 
}


//-------------------------------------- sending data to Google Sheets
void sendData(float tem, int hum) {
  Serial.println("==========");
  Serial.print(F("connecting to "));
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println(F("connection failed"));
    return;
  }

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print(F("requesting URL: "));
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println(F("request sent"));

  //----------------------------------------Checking whether the data was sent successfully or not, for debug
  /*while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println(F("headers received"));
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println(F("esp8266/Arduino CI successfull!"));
  } else {
   Serial.println(F("esp8266/Arduino CI has failed"));
  }*/
  //Serial.print(F("reply was : "));
  //Serial.println(line);
  //Serial.println(F("closing connection"));
  //Serial.println(F("=========="));
  //Serial.println();
  return;
} 
//==============================================================================
