//OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED define
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>

// Set these to run example.
#define FIREBASE_HOST "url"
#define FIREBASE_AUTH "auth"
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "pass"
WiFiUDP udp;

EasyNTPClient ntpClient(udp, "pool.ntp.org", (-1*(4*60*60))); // IST = GMT -4:00
long int previus = 0;

#include "MQ135.h"
#define ANALOGPIN A0    //  Define Analog PIN on Arduino Board
#define RZERO 206.85    //  Define RZERO Calibration Value
MQ135 gasSensor = MQ135(ANALOGPIN);
void setup() {

    Serial.begin(9600);
  Serial.println(ntpClient.getUnixTime());

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  previus = ntpClient.getUnixTime();

  float rzero = gasSensor.getRZero();
  Serial.print("MQ135 RZERO Calibration Value : ");
  Serial.println(rzero);
  
  // put your setup code here, to run once:
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display(); //Display logo
  delay(1000); 
  display.clearDisplay();
}
int n = 0;
//int interval = 300;
int interval = 60;
float ppm = 0;
void loop() {
  // put your main code here, to run repeatedly:
 display.clearDisplay();
 oledDisplayHeader();

  long int current =  ntpClient.getUnixTime();
  if(current-previus>interval){
      previus =  ntpClient.getUnixTime();
      ppm = gasSensor.getPPM();
      String pathId = "ppm/"+String(ntpClient.getUnixTime())+"/co2";
      String name = Firebase.pushFloat(pathId, ppm);
      // handle error
      if (Firebase.failed()) {
          Serial.print("pushing "+pathId+" failed:");
          Serial.println(Firebase.error());  
          return;
      }
      Serial.print("pushed: /"+pathId+"/");
      Serial.println(ppm);
  }

 oledDisplay(3,5,28,ppm,"");
 //oledDisplay(2,70,16,0,"C");
 //oledDisplay(2,70,44,0,"F");
 
 display.display(); 
}


void oledDisplayHeader(){
 display.setTextSize(1);
 display.setTextColor(WHITE);
 display.setCursor(0, 0);
 display.print("Calidad de aire");
 display.setCursor(60, 0);
 display.print("");
}
void oledDisplay(int size, int x,int y, float value, String unit){
 int charLen=12;
 int xo=x+charLen*3.2;
 int xunit=x+charLen*3.6;
 int xval = x; 
 display.setTextSize(size);
 display.setTextColor(WHITE);
 
 if (unit=="%"){
   display.setCursor(x, y);
   display.print(value,0);
   display.print(unit);
 } else {
   if (value>99){
    xval=x;
   } else {
    xval=x+charLen;
   }
   display.setCursor(xval, y);
   display.print(value,0);
   //display.drawCircle(xo, y+2, 2, WHITE);  // print degree symbols (  )
   display.setCursor(xunit, y);
   display.print(unit);
 }
 
}