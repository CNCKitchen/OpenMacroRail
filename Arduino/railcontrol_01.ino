#include <SpeedyStepper.h>

const int SHUTTER_PIN = 16;
const int MOTOR_STEP_PIN = 18;
const int MOTOR_DIRECTION_PIN = 19;

///SHooting setting
const float DISTANCE = 2; //mm
const int STEPS = 200;
const float SPEED = 0.1; //mm/s
const int DESHAKEPAUSE = 4; //s


SpeedyStepper stepper;

void setup() 
{
  //
  // setup the LED pin and enable print statements
  //
  pinMode(SHUTTER_PIN, OUTPUT);  
  digitalWrite(SHUTTER_PIN, LOW); 
  Serial.begin(9600);

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
}



void loop() 
{

  stepper.setStepsPerMillimeter(3200);    // 8x microstepping, 2/1 gear reduction
  stepper.setSpeedInMillimetersPerSecond(SPEED);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(1.0);



  //
  // Use "relative" positioning to make several more moves, 1mm each
  //
float currentPos = 0.0;
float increment = DISTANCE / STEPS;
  
  while(currentPos < DISTANCE)
  {
    // shoot a frame
    delay(DESHAKEPAUSE * 1000);
    digitalWrite(SHUTTER_PIN, HIGH);
    delay(200);
    digitalWrite(SHUTTER_PIN, LOW);
    delay(500);
    //increment position
    stepper.moveRelativeInMillimeters(increment);
    currentPos += increment;
    Serial.print("Current position: ");
    Serial.print(currentPos);
    Serial.println(" mm");
  }

 delay(300000);

}
