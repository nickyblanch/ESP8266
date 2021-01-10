/*
 * HTTPS Secured Client GET Request
 * Copyright (c) 2019, circuits4you.com
 * All rights reserved.
 * https://circuits4you.com 
 * Connects to WiFi HotSpot.
 * ADDITIONAL CODE BY NICK BLANCHARD */

// ESP8266 AND SI701 LIBRARIES
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
//#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "SparkFun_Si7021_Breakout_Library.h"

// WiFi Network Credentials
const char *ssid = "Cottages at Tucson";  //ENTER YOUR WIFI SETTINGS
const char *password = "passw0rd";

// Web server to send HTTPS requests to
const char *host = "www.nickyblanch.xyz";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "49 47 81 0F F9 1B 21 30 22 A7 97 64 DF 43 1F 5F 5E 30 9B 95";

// SI701 VARIABLES AND PINS
float humidity = 0;
float tempf = 0;
Weather sensor;


//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {

  // Enable debugging light
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // Configure ESP8266
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot

  // Connect to WiFi network
  WiFi.begin(ssid, NULL);
  Serial.println("");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

   // Find ESP8266 physical address
  Serial.println("");
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());


  // Begin SI701 Sensor
  sensor.begin();
  digitalWrite(2, LOW);
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
    digitalWrite(2, HIGH);
    return;
  }
  else {
    Serial.println("Connected to web");
  }
  
  String Link;
  
  //GET Data
  get_weather();

  // Append data to HTTPS Get link
  Link = "/test.php?temp=" + String(tempf) + "&humidity=" + String(humidity);

  Serial.print("requesting URL: ");
  Serial.println(host+Link);

  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");
    
  delay(5 * 60000);  //GET Data at every 5 minutes
}


void get_weather() {
  // Measure humidity
  humidity = sensor.getRH();

  // Measure temperature
  tempf = sensor.getTempF();
}
