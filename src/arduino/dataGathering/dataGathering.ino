//Required for SPI bus
#include <SPI.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
//contains wifi details
#include "arduino_secrets.h"

//Chip select
#define CS1    0
#define CS2    2
//Debug
#define DEBUG  0

//wifi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;    //WiFi radio status

// variables for lightData function
int i;
int recl[2];
int lumen;

// variables for noiseData function
int j;
byte recn[3];
int X;
long sum;

//JSON
//JSON stack allocation
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();


void setup() {
  //init serial connection
  Serial.begin(9600);
  if (DEBUG) delay(5000);
  // Set chip select pin directions
  // other pin settings are automatic
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);

  // Init SPI
  SPI.begin();

  //set SPI mode
  SPI.setDataMode(SPI_MODE0);
  //set clock divider
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  //check for the presence of the wifi shield
  //in case the chip breaks or version mismatch
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Wifi shield broken, or version mismatch.");
    while(true);
  }
  Serial.println("Connecting to WiFi with WPA2");
  status = WiFi.begin(ssid, pass);
  if(status != WL_CONNECTED) {
    Serial.println("couldn't connect");
    while(true);
  }
  else {
    Serial.println("connected to wifi");
  }
}

void checkNetworks() {
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)Serial.println("Couldn't get a wifi connection");
}

void lightData() {
  digitalWrite(CS1, LOW);
  for(i=0;i<2;i=i+1){
    recl[i] = SPI.transfer(0);
  }
  digitalWrite(CS1, HIGH);
  if(DEBUG) {
    for (i=0;i<2;i++) {
      Serial.print(i);
      Serial.print("=");
      Serial.println(recl[i]);
    }
  }
  lumen=(recl[0]<<3)|(recl[1]>>4);
  //Serial.print("lumens = ");
  //Serial.println(lumen);
  delay(10);
}

void noiseData() {
  digitalWrite(CS2, LOW);
  delayMicroseconds(20);
  for (i=0;i<2;i++) {
    recn[i] = SPI.transfer(0);
    delayMicroseconds(20);
  }
  digitalWrite(CS2, HIGH);
  X = recn[1];
  X|= (recn[0] << 8);

  for(int i=0; i<32; i++) {
    sum = sum + X;
  }

  sum >>=5;
  if(DEBUG) Serial.println(sum);
  delay(10);
  
}

void createJson() {
  root["light"] = lumen;
  root["noise"] = sum;
}

void loop() {
  // put your main code here, to run repeatedly:
lightData();
noiseData();
createJson();
root.printTo(Serial);
Serial.println();
delay(1000);
}
