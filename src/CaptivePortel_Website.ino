
#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ESPmDNS.h>

const char* ssid = "IZIG_Netowork"; //Name of the WIFI network hosted by the device
const char* password =  "";               //Password

AsyncWebServer server(80);                //This creates a web server, required in order to host a page for connected devices

DNSServer dnsServer;                      //This creates a DNS server, required for the captive portal


void initSDCard(){
  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}


void webServerSetup(){
  

  //This is a super simple page that will be served up any time the root location is requested.  Get here intentionally by typing in the IP address.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/VideoPage.html", "text/html");
  });


  //This is an example of triggering for a known location.  This one seems to be common for android devices
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/ZooPage.html", "text/html");
    Serial.println("requested /generate_204");
  });

  //This is an example of a redirect type response.  onNotFound acts as a catch-all for any request not defined above
  server.onNotFound([](AsyncWebServerRequest *request){
    request->redirect("/generate_204");
    Serial.print("server.notfound triggered: ");
    Serial.println(request->url());       //This gives some insight into whatever was being requested
  });
  server.serveStatic("/", SD, "/");
  server.begin();                         //Starts the server process
  Serial.println("Web server started");
}




void setup() {
  
  Serial.begin(115200);

  WiFi.softAP(ssid, password);            //This starts the WIFI radio in access point mode
  Serial.println("Wifi initialized");
  
  Serial.println(WiFi.softAPIP());        //Print out the IP address on the serial port (this is where you should end up if the captive portal works)
    if (!MDNS.begin("IZIG")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  initSDCard();
  dnsServer.start(53, "*", WiFi.softAPIP());  //This starts the DNS server.  The "*" sends any request for port 53 straight to the IP address of the device

  webServerSetup();                       //Configures the behavior of the web server
  
  Serial.println("Setup complete");
}


void loop() {
  dnsServer.processNextRequest();         //Without this, the connected device will simply timeout trying to reach the internet
                                          //or it might fall back to mobile data if it has it
  
}
