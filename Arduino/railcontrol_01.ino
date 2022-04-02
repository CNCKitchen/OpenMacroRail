#include <SpeedyStepper.h>

// const int LED_PWM_PIN = 4;
// const int LIMIT1_PIN = 12;
// const int LIMIT2_PIN = 13;
// const int ENC_A_PIN = 32;
// const int ENC_B_PIN = 35;
// const int STEP_INDEX_PIN = 25;
const int SHUTTER_PIN = 5;
const int MOTOR_STEP_PIN = 18;
const int MOTOR_DIRECTION_PIN = 19;
const int STEP_EN_PIN = 26;
const int STEP_MS1_PIN = 27;
const int STEP_MS2_PIN = 14;
const int STEP_DIAG_PIN = 33;

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
  pinMode(STEP_EN_PIN, OUTPUT); 
  pinMode(STEP_MS1_PIN, OUTPUT); 
  pinMode(STEP_MS2_PIN, OUTPUT);  
  digitalWrite(SHUTTER_PIN, LOW); 
  digitalWrite(STEP_EN_PIN, LOW); 
  digitalWrite(STEP_MS1_PIN, LOW); 
  digitalWrite(STEP_MS2_PIN, LOW); 
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
  
  while((increment > 0 && currentPos < DISTANCE) || (increment < 0 && currentPos > DISTANCE ))
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
