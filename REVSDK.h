#include "DcMotor.h"
#include "ElapsedTime.h"

#define START_BUTTON_PIN 8
#define UPDATE_DELAY 10

HardwareWire hardWire;
DcExpansion motors(5, &hardWire);

void runOpMode();

uint32_t oldUpdateTime = 0;

bool opModeIsActive(){
  while(millis() - oldUpdateTime < UPDATE_DELAY);

  oldUpdateTime = millis();

  return true;
}

void waitForStart(){
  while(digitalRead(START_BUTTON_PIN));
}

void telemetryUpdate(){
  Serial.println();
}

void telemetryAddData(String name, int data){
  Serial.print(name);
  Serial.print(": ");
  Serial.print(data);
  Serial.print(", ");
}

void telemetryAddData(String name, String data){
  Serial.print(name);
  Serial.print(": ");
  Serial.print(data);
  Serial.print(", ");
}

void telemetryAddData(String name, int32_t data){
  telemetryAddData(name, data);
}

void telemetryAddData(String name, float data){
  Serial.print(name);
  Serial.print(": ");
  Serial.print(data);
  Serial.print(", ");
}

void telemetryAddData(String name, bool data){
  Serial.print(name);
  Serial.print(": ");
  Serial.print(data);
  Serial.print(", ");
}

void telemetryAddStr(String str){
  Serial.print(str);
  Serial.print(", ");
}

void setup(){
  hardWire.begin();
  motors.begin();

  Serial.begin(9600);

  pinMode(START_BUTTON_PIN, INPUT);

  runOpMode();
}

void loop(){

}