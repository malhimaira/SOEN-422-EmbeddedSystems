#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Servo.h>


int forw_FL = 21; // H-bridge leg 1
int back_FL = 13; // H-bridge leg 2
int forw_FR = 14; // H-bridge leg 1
int back_FR = 12; // H-bridge leg 2
int back_RL = 16; // H-bridge leg 2forw_RL
int forw_RL = 17; // H-bridge leg 2
int back_RR = 2; // H-bridge leg 2
int forw_RR = 4; // H-bridge leg 2

int 

int right = 22; 
int left = 25; 
int buzz = 15;

int speed_value_motor1; // value for motor speed
bool destinationReached =false;
int trigPin = 18;
int echoPin = 23;
long duration;
int distance;

//Initialize all fo the objects that are needed.
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
String rxString = "";
bool convertInput = false;


#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//Initialize servo responsible for locking/unclocking the door.
Servo fan;
int fanPin = 27;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  //Triggered when user sends signal to the devices
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      //If the length is >0, we know the user sent data.
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        convertInput = true;
        //Store the input from the device so we can then check what the user wants to accomplish.
        for (int i = 0; i < rxValue.length(); i++)
        {
          Serial.print(rxValue[i]);
          rxString = rxString + rxValue[i];                    
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  pinMode(25, INPUT);
  pinMode(22, INPUT);
  pinMode(34, INPUT); //IR 

  pinMode(buzz, OUTPUT); 

 pinMode(forw_FL, OUTPUT);
 pinMode(back_FL, OUTPUT);
 pinMode(forw_FR, OUTPUT);
 pinMode(back_FR, OUTPUT);
 pinMode(forw_RL, OUTPUT);
 pinMode(back_RL, OUTPUT);
 pinMode(forw_RR, OUTPUT);
 pinMode(back_RR, OUTPUT);

 pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // Create the BLE Device
  BLEDevice::init("JosephTTGO");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);

  //Add the descriptor   
  pTxCharacteristic->addDescriptor(new BLE2902());

  //Create a characteristic
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  fan.attach(fanPin);
  Serial.begin(115200);
 
}

void loop() {
if(!destinationReached)
{
  determinePosition();
}
else
{
    if (deviceConnected) {
        //pTxCharacteristic->setValue(&txValue, 1);
        //pTxCharacteristic->notify();
        //txValue = ;
		  delay(10); // bluetooth stack will go into congestion, if too many packets are sent
	    }
      // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

//If the user input anything we need to turn it into an action.
    if(convertInput)
    {
      Serial.println("CONVERTING");
      convertControlpad();
    }
}
}

void determinePosition() {

  //Get the value of sensor
  int sensorL = digitalRead(left); //LEFT SENSOR
  int sensorR = digitalRead(right); //RIGHT SENSOR

     
  distance = determineDistance();
  delay(50);
  while(distance<=15){
    tone(buzz, 500, 200);
    delay(200);
    Serial.print("Distance: ");
    Serial.println(distance);
    stopMoving();
    noTone(buzz);
    distance = determineDistance();

  }
  if(sensorL == 0 && sensorR ==0){
    moveForward();

  }
  else if (sensorL == 1 && sensorR == 0){
      // Case 2:  black line found on left sensor , means turn LEFT until both sensors see white
      turnLeft();
      
  }
  else if (sensorL == 0 && sensorR == 1){
      // Case 3:  black line found on right sensor , means turn RIGHT until both sensors see white
      turnRight();
  }
  else if (sensorL == 1 && sensorR == 1){
    // Case 4: Destination reached
    // TO DO : code to stop car
    destinationReached = true;
    stopMoving();
  }
}

// This method converts what is sent by bluetooth into actions. For example, turning lights on/off or playing music
void convertControlpad() {
  Serial.print("   test   ");
  if (rxString == "!B11:") startFan();//playSong1();
  //else if (rxString == "!B219") //playSong2();
  //else if (rxString == "!B318") //toggleGreenLightOn();
  //else if (rxString == "!B417") //toggleRedLightOn();
  else if (rxString == "!B516") moveForward();
  else if (rxString == "!B615") moveBackwards();
  else if (rxString == "!B714") turnLeft();
  else if (rxString == "!B813") turnRight();
  //else if (rxString == "!B309") //toggleGreenLightOff();
  //else if (rxString == "!B408") //toggleRedLightOff();
  else if (rxString == "!B507") 
  {
    Serial.println("STOP MOVING!");
    stopMoving(); //Forward off
  }
  else if (rxString == "!B606") stopMoving(); //Backwards off
  else if (rxString == "!B705") stopMoving(); //Left off
  else if (rxString == "!B804") stopMoving();  //Right off

  //We need to set convertInput back to false in order to trigger this again for the next action.
  convertInput = false;
  //Reset the rxString back to empty to capture the next user's input
  rxString = "";
}


void moveForward(){
 digitalWrite(forw_FL, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_FL, LOW); // set leg 2 of the H-bridge high
 digitalWrite(forw_FR, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_FR, LOW); // set leg 2 of the H-bridge high
 digitalWrite(forw_RL, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_RL, LOW); // set leg 2 of the H-bridge high  
 digitalWrite(forw_RR, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_RR, LOW); // set leg 2 of the H-bridge high
}

void moveBackwards(){
 digitalWrite(forw_FL, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_FL, HIGH); // set leg 2 of the H-bridge highHIGH
 digitalWrite(forw_FR, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_FR, HIGH); // set leg 2 of the H-bridge high
 digitalWrite(forw_RL, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_RL, HIGH); // set leg 2 of the H-bridge high  
 digitalWrite(forw_RR, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_RR, HIGH); // set leg 2 of the H-bridge high  
}

void stopMoving()
{
 digitalWrite(forw_FL, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_FL, LOW); // set leg 2 of the H-bridge high
 digitalWrite(forw_FR, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_FR, LOW); // set leg 2 of the H-bridge high
 digitalWrite(forw_RL, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_RL, LOW); // set leg 2 of the H-bridge high  
 digitalWrite(forw_RR, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_RR, LOW); // set leg 2 of the H-bridge high
}

void turnRight(){
 digitalWrite(forw_FL, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_FL, LOW); // set leg 2 of the H-bridge high

 digitalWrite(forw_FR, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_FR, HIGH); // set leg 2 of the H-bridge high
 
 digitalWrite(forw_RL, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_RL, LOW); // set leg 2 of the H-bridge high  

 digitalWrite(forw_RR, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_RR, HIGH); // set leg 2 of the H-bridge high
}

void turnLeft(){
 digitalWrite(forw_FL, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_FL, HIGH); // set leg 2 of the H-bridge high

 digitalWrite(forw_FR, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_FR, LOW); // set leg 2 of the H-bridge high

 digitalWrite(forw_RL, LOW); // set leg 1 of the H-bridge low
 digitalWrite(back_RL, HIGH); // set leg 2 of the H-bridge high 
  
 digitalWrite(forw_RR, HIGH); // set leg 1 of the H-bridge low
 digitalWrite(back_RR, LOW); // set leg 2 of the H-bridge high
}

int determineDistance(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  // Serial.print("Distance: ");
  // Serial.println(distance);
  // Serial.println(analogRead(34));
  return distance;
}

void startFan()
{
  for (int pos = 0; pos <= 180; pos += 10) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    fan.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (int pos = 180; pos >= 0; pos -= 10) { // goes from 180 degrees to 0 degrees
    fan.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

