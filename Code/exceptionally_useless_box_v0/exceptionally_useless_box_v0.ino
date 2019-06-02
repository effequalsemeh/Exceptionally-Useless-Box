///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This code is for The Exceptionally Useless Box. Currently set up to be run on an Arduino Pro Mini.
// Written by: Kyle Fitch
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Related project files are hosted on this repository: https://github.com/effequalsemeh/Exceptionally-Useless-Box
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Version History:
// 6-2-19 v0: initial release
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A note on using this code:
// When I wrote this, I did so in 30 minutes just to get it working. There are far more efficient ways to go about this,
// but this was the quickest and dirtiest way I could get it up and running. When the device is mechanically set up and in
// position, the botAngleMax, botAngleMin, topAngleMax, and topAngleMin need to be tuned to dial in on how hard the arms need
// to press the switch and toggle it over.

#include <Servo.h>

const int switchPin = 5; // pin that the toggle switch is connected to
const int trigPin = 6; // "trig" pin on the ultrasonic sensor
const int echoPin = 7; // "echo" pin on the ultrasonic sensor
Servo topServo; // the "top" servo is the one whose arms extend out of the top lid
Servo botServo; // the "bot" or bottom servo is the one whose arms extend out of the front lid

float distance = 0; // variable to store the measured distance from the ultrasonic sensor
int switchState = 0; // variable to store the state of the switch
boolean switching = false; // used to control the state of the device and handle the delay of the servo rotation

const int botAngleMax=7;                 //angle in degrees that the bottom arm will fully extend to
const int botAngleMin=180;        // angle in degrees that the bottom arm goes to when fully retracted
const int topAngleMax=18;                //angle in degrees that the top arm will fully extend to        
const int topAngleMin=170;               //angle in degrees that the top arm goes to when fully retracted
const int distanceMax=50;                //the maximum distance away from the sensor that the bottom servo will respond to
const float thresholdPercent = 0.93; // percentage of the maximum travel of the bottom arm where the arm should fully commit to pressing the switch
const float threshCommit = thresholdPercent * botAngleMin; // the calculated angle when the arm should fully commit to pressing the switch


void setup() 
{
  topServo.attach(11);              
  botServo.attach(10);              
  topServo.write(topAngleMin);        //set initial position of the top arm
  botServo.write(botAngleMin);       //set initial position of the bottom arm
  pinMode(switchPin, INPUT_PULLUP); // don't forget to put a pull-up resistor on this pin
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Device is ready");

  // if the switch was already on when the device powers on, turn the switch off before entering the main loop
  if (digitalRead(switchPin)==HIGH && switchState == 0) // check if the switch is turned on when it was previously off
  {
    Serial.println("Upon boot, the switch was seen as turned on. Resetting it...");
    switching = true;
    switchState = 1;
    topServo.write(topAngleMax); // set position of top arm to hit the switch back off
    botServo.write(botAngleMin); // retract the bottom arm while the top arm turns the switch off
    while (switching == true) // don't do anything except wait for the arm to hit the switch 
    {
      if (digitalRead(switchPin) == LOW) // check if the switch is off yet
      {
        switchState = 0;
        switching = false;
        topServo.write(topAngleMin); // retract the top arm now that the switch is off
      }
    }
  }
  Serial.println("Starting the main loop");
}

void loop() 
{
  if (digitalRead(switchPin) == HIGH) // check if the switch was changed by the user before the bottom arm could do it
  {
    Serial.println("The switch was toggled before device was able to do it");
    switching = true;
    resetSwitch(); 
  }
  distance = getDistance();
  if (distance < distanceMax) // if the reading is close enough to the device, update the motor position
  {
    updateBotArm(distance);
    delay(15);                                                                                                                                            
  }
  else
  {
    botServo.write(botAngleMin); // retract the bottom arm if there's nothing in the way of the sensor
  }
}

int getDistance()
{                                                                                                                               
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  float duration = pulseIn(echoPin, HIGH);
  float measuredDistance = duration*.034/2;                                                                       
  return measuredDistance;
}


void updateBotArm(int handDistance)
{
  float motorPosition = map(handDistance, 0, distanceMax, botAngleMax, botAngleMin); // convert the sensor distance range to the output angle range, but inverted so that the angle goes up as the distance goes down
      Serial.print(distance);
    Serial.print(", ");
    Serial.println(motorPosition);
  {
    if (distance < 7)
    {
      commit();
    }
    else
    {
      botServo.write(motorPosition);
    }
  }
}

void commit()
{
  Serial.println("Committing to pressing the switch...");
  botServo.write(botAngleMax);
  while (switchState == 0)
  {
    if (digitalRead(switchPin) == HIGH)
    {
      switchState = 1;
      switching = true;
      botServo.write(botAngleMin);
      Serial.println("Switch has been toggled by the bottom arm");
    }
  }
  resetSwitch();
}

void resetSwitch()
{
  Serial.println("Resetting switch...");
  topServo.write(topAngleMax); // set position of top arm to hit the switch back off
  while (switching == true) // don't do anything except wait for the arm to hit the switch 
  {
    if (digitalRead(switchPin) == LOW) // check if the switch is off yet
    {
      switchState = 0;
      switching = false;
      topServo.write(topAngleMin); // retract the top arm now that the switch is off
      Serial.println("Switch has been toggled back by the top arm");
    }
  }
  Serial.println("Switch should be reset now");
}

