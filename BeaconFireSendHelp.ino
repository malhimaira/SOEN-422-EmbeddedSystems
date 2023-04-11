
//Joseph Mezzacappa (40137899), Jonathan Jong (40133041), Maira Malhi (40128269)
//SOEN 422 Final Project
//Description: Fire beacon to request help from fire department. Changes value in the Ubidots cloud database to send request for help (to send the fire vehicle). 

#include <WiFi.h>
#include "Ubidots.h"


const char* UBIDOTS_TOKEN = "BBFF-Akl1gWlT3i3zlhQn3wVi27kNeRsGch";  //  Ubidots TOKEN associated with account
const char* WIFI_SSID = "VIDEOTRON4406-EXT";      //  Wi-Fi SSID
const char* WIFI_PASS = "lanhalen";      // Wi-Fi password
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP); //Using HTTP 


void setup() {
  pinMode(38,INPUT); //Initialize the pin for the flame sensor
  Serial.begin(115200); //Begin serial monitor 
  WiFi.mode(WIFI_STA); //Connect to wifi access point
  WiFi.begin(WIFI_SSID, WIFI_PASS); //credentials
  Serial.println("");
  ubidots.setDebug(true); //Ubidots debug 

  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS); // for Ubidots to connect using credentials given

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID); //Show the connected SSID 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //Show the IP address
  


}

void loop(){

  Serial.println("This is the flame sensor reading");
  Serial.println(digitalRead(38)); //Display the flame sensor reading (0 for no flame, 1 for flame)
  ubidots.add("firestatus",digitalRead(38)); //Adding fire status variable to the Ubidots database
  ubidots.send();  // Send data to the database
  delay(5000);  //Delay for processing
}