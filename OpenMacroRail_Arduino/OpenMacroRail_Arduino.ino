#include <WiFiClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SpeedyStepper.h>
#include <Preferences.h>
#include "WiFi-credentials.h"

// Unused pin declarations (these pins are present on Motorized Macro Slider PCB V1.2):
// const int LED_PWM_PIN = 4;
// const int LIMIT1_PIN = 12;
// const int LIMIT2_PIN = 13;
// const int ENC_A_PIN = 32;
// const int ENC_B_PIN = 35;
// const int STEP_INDEX_PIN = 25;
// HardwareSerial &serial_stream = Serial2; //Serial2: RX pin 16, TX pin 17

// Pin declarations
const int SHUTTER_PIN = 5;
const int MOTOR_STEP_PIN = 18;
const int MOTOR_DIRECTION_PIN = 19;
const int STEP_EN_PIN = 26;
const int STEP_MS1_PIN = 27;
const int STEP_MS2_PIN = 14;
const int STEP_DIAG_PIN = 33;

// Adjustable settings (through web interface)
float startPoint = 0; //mm
float endPoint = 0; //mm
float jogIncrement = 0.1; //mm
int numImages = 200;

// Adjustable settings (through web interface) that will be stored in EEPROM between power-cycles
Preferences preferences; //Used to store user-settings in EEPROM
float shootingSpeed = 0.1; //mm/s
float jogSpeed = 1; //mm/s
float deshakeDelay = 4; //s
float shootDelay = 0.5; //s
float overshootDistance = 0.5; //mm (used by goToStartPoint() to counteract mechanical backlash by making sure gear teeth are active from the beginning.)

//Calculated settings:
float currentPos = 0.0; //mm
float distance = 0.0; //mm
float increment = 0.0; //mm
float remainingShootingTime = 0.0; //min
float totalShootingTime = 0.0; //min
int picsTaken = 0; 
int remainingPictures = 0; 

SpeedyStepper stepper;

//Motor and mechanical parameters
const float MOTOR_STEP_ANGLE = 1.8; // Degrees revolution per full step (motor parameter)
const uint32_t FULLSTEPS_PER_REVOLUTION = 360.0/MOTOR_STEP_ANGLE; // Number of steps per full revolution. 360/1.8 = 200
const uint32_t GEAR_REDUCTION_FACTOR = 2; // Mechanical reduction between motor and slider. 2:1 default
uint16_t microsteps = 8;                  //Default value 8 microsteps
uint32_t microstepsPerMillimeter = microsteps * FULLSTEPS_PER_REVOLUTION * GEAR_REDUCTION_FACTOR; // 8x microstepping, 200 steps per revolution, and 2:1 gear reduction gives 3200

//Flags
volatile bool JOGFWDFLAG = false;
volatile bool JOGBWDFLAG = false;

enum OPERATING_STATE{
  JOGGING,
  PHOTO_SESSION,
  EMERGENCY_STOP,
  BACKLASH_SWEEP,
  BACKLASH_DETECTED,
  HOMING,
  HOME_DETECTED,
  TEST_MODE
};
OPERATING_STATE current_state = JOGGING;

WebServer server(80);

const char *webpage = 
#include "webPage.h"
;

void handleRoot() {

  server.send(200, "text/html", webpage);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void jog_fwd(){
  JOGFWDFLAG = false;
  stepper.moveRelativeInMillimeters(jogIncrement);
  currentPos += jogIncrement;
}

void jog_bwd(){
  JOGBWDFLAG = false;
  stepper.moveRelativeInMillimeters(-jogIncrement);
  currentPos -= jogIncrement;
}

void activateCameraTrigger(){
  digitalWrite(SHUTTER_PIN, HIGH);
  delay(200);
  digitalWrite(SHUTTER_PIN, LOW);
  picsTaken +=1;
}

void goToStartPoint(){
  float currentMove;
  //TODO: Manual overshoot should be replaced with load sensing through TMC SafeGuard function.
  if((currentPos - startPoint)<0){
    currentMove = -(currentPos - startPoint) + overshootDistance;
    stepper.moveRelativeInMillimeters(currentMove);
    currentPos += currentMove;
    stepper.moveRelativeInMillimeters(-overshootDistance);
    currentPos -= overshootDistance;
  }else{
    currentMove = -(currentPos - startPoint) - overshootDistance;
    stepper.moveRelativeInMillimeters(currentMove);
    currentPos += currentMove;
    stepper.moveRelativeInMillimeters(overshootDistance);
    currentPos += overshootDistance;
  }
}

void calculateStats(){
  increment = distance/(numImages-1);
  remainingPictures = numImages-picsTaken;
  if(distance>0){
    totalShootingTime = (numImages*(shootDelay+deshakeDelay+0.2)+distance/shootingSpeed)/60; //Simplified calculation that ignores accelleration
  }else{
    totalShootingTime = 0;
  }
  if(current_state==OPERATING_STATE::PHOTO_SESSION){
    remainingShootingTime = (remainingPictures*(shootDelay+deshakeDelay+0.2)+distance/shootingSpeed)/60; //Simplified calculation that ignores accelleration
  }else{
    remainingShootingTime = 0;
  }
}

void setStartPoint(){
  if (current_state != PHOTO_SESSION)
  {
    startPoint = currentPos;
    distance = endPoint-startPoint;
    calculateStats();
  }
}

void setEndPoint(){
  if (current_state != PHOTO_SESSION)
  {
    endPoint = currentPos;
    distance = endPoint-startPoint;
    calculateStats();
  }
}

void updateStepperSettings(){
  stepper.setStepsPerMillimeter(microstepsPerMillimeter);
  if(current_state==JOGGING){
    stepper.setSpeedInMillimetersPerSecond(jogSpeed);
  }else{
    stepper.setSpeedInMillimetersPerSecond(shootingSpeed);
  }
  stepper.setAccelerationInMillimetersPerSecondPerSecond(1.0);
  calculateStats();
}

void writeSettingsToEEPROM(){
  preferences.putFloat("shootingSpeed",shootingSpeed);
  preferences.putFloat("jogSpeed",jogSpeed);
  preferences.putFloat("deshakeDelay",deshakeDelay);
  preferences.putFloat("shootDelay",shootDelay);
  preferences.putFloat("overshootDist",overshootDistance);
  preferences.putUInt("init",1); //Indicate that EEPROM contains data for next reboot
}

void readSettingsFromEEPROM(){
  if(preferences.getUInt("init",0) != 0){
    shootingSpeed     = preferences.getFloat("shootingSpeed",shootingSpeed);
    jogSpeed          = preferences.getFloat("jogSpeed",jogSpeed);
    deshakeDelay      = preferences.getFloat("deshakeDelay",deshakeDelay);
    shootDelay        = preferences.getFloat("shootDelay",shootDelay);
    overshootDistance = preferences.getFloat("overshootDist",overshootDistance);
  }else{
    writeSettingsToEEPROM();
  }
}

void setup(void){

  pinMode(SHUTTER_PIN, OUTPUT); 
  pinMode(STEP_EN_PIN, OUTPUT); 
  pinMode(STEP_MS1_PIN, OUTPUT); 
  pinMode(STEP_MS2_PIN, OUTPUT);  
  digitalWrite(SHUTTER_PIN, LOW); 
  digitalWrite(STEP_EN_PIN, LOW); 
  digitalWrite(STEP_MS1_PIN, LOW); // Note: Modifying MS1 or MS2 will change UART address of TMC  
  digitalWrite(STEP_MS2_PIN, LOW); // Note: Modifying MS1 or MS2 will change UART address of TMC

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  preferences.begin("settings",false);
  readSettingsFromEEPROM();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("OpenMacroRail-GUI")) {
    Serial.println("MDNS Responder Started");
  }

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  updateStepperSettings();

  server.on("/", handleRoot);

  server.on("/jogFwd", [](){
    Serial.println("jogFwd");
    JOGFWDFLAG = true;
    server.send(200, "text/plain", "jogFwd");
  });

  server.on("/jogBwd", [](){
    Serial.println("jogBwd");
    JOGBWDFLAG = true;
    server.send(200, "text/plain", "jogBwd");
  });

  server.on("/jogIncrSlow", [](){
    Serial.println("jogIncrSlow");
    jogIncrement = 0.1;
    updateStepperSettings();
    server.send(200, "text/plain", "jogIncrSlow");
  });

  server.on("/jogIncrMedium", [](){
    Serial.println("jogIncrMedium");
    jogIncrement = 0.5;
    updateStepperSettings();
    server.send(200, "text/plain", "jogIncrMedium");
  });

  server.on("/jogIncrFast", [](){
    Serial.println("jogIncrFast");
    jogIncrement = 1.0;
    updateStepperSettings();
    server.send(200, "text/plain", "jogIncrFast");
  });

  server.on("/setStartPoint", [](){
    Serial.println("setStartPoint");
    setStartPoint();
    server.send(200, "text/plain", "setStartPoint");
  });

  server.on("/setEndPoint", [](){
    Serial.println("setEndPoint");
    setEndPoint();
    server.send(200, "text/plain", "setEndPoint");
  });

  server.on("/testImage", [](){
    Serial.println("testImage");
    activateCameraTrigger();
    server.send(200, "text/plain", "testImage");
  });
  
  server.on("/start", [](){
    Serial.println("start");
    goToStartPoint();
    current_state=PHOTO_SESSION;
    picsTaken = 0;
    updateStepperSettings();
    server.send(200, "text/plain", "start");
  });

  server.on("/abort", [](){
    Serial.println("abort");
    current_state=JOGGING;
    updateStepperSettings();
    server.send(200, "text/plain", "abort");
  });

  server.on("/numImagesForm", [](){
    Serial.println("numImagesForm");
    if(server.hasArg("num")){
      numImages = server.arg("num").toInt();
      calculateStats();
    }
    server.send(200, "text/plain", String(numImages));
  });

  server.on("/shootingSpeedForm", [](){
    Serial.println("shootingSpeedForm");
    if(server.hasArg("num")){
      shootingSpeed = server.arg("num").toFloat();
      updateStepperSettings();
      preferences.putFloat("shootingSpeed",shootingSpeed);
    }
    server.send(200, "text/plain", String(shootingSpeed));
  });

  server.on("/jogSpeedForm", [](){
    Serial.println("jogSpeedForm");
    if(server.hasArg("num")){
      jogSpeed = server.arg("num").toFloat();
      updateStepperSettings();
      preferences.putFloat("jogSpeed",jogSpeed);
    }
    server.send(200, "text/plain", String(jogSpeed));
  });

  server.on("/deshakeDelayForm", [](){
    Serial.println("deshakeDelayForm");
    if(server.hasArg("num")){
      deshakeDelay = server.arg("num").toFloat();
      calculateStats();
      preferences.putFloat("deshakeDelay",deshakeDelay);
    }
    server.send(200, "text/plain", String(deshakeDelay));
  });

  server.on("/shootDelayForm", [](){
    Serial.println("shootDelayForm");
    if(server.hasArg("num")){
      shootDelay = server.arg("num").toFloat();
      calculateStats();
      preferences.putFloat("shootDelay",shootDelay);
    }
    server.send(200, "text/plain", String(shootDelay));
  });

  server.on("/overshootDistanceForm", [](){
    Serial.println("overshootDistanceForm");
    if(server.hasArg("num")){
      overshootDistance = server.arg("num").toFloat();
      calculateStats();
      preferences.putFloat("overshootDist",overshootDistance);
    }
    server.send(200, "text/plain", String(overshootDistance));
  });
  
  server.on("/refreshStats", [](){
    calculateStats();
    String response = "startPoint:"+String(startPoint,3)+" mm,"+"endPoint:"+String(endPoint,3)+" mm,"+"currentPos:"+String(currentPos,3)+" mm,"+"distance:"+String(distance)+" mm,"+"increment:"+String(increment,4)+" mm,"+"remainingPictures:"+String(remainingPictures)+","+"remainingShootingTime:"+String(remainingShootingTime)+" min,"+"totalShootingTime:"+String(totalShootingTime)+" min,";
    server.send(200, "text/plain", response);
  });
  
  server.on("/refreshForms", [](){
    String response = "jogIncrement:"+String(jogIncrement,1)+","+"numImages:"+String(numImages)+","+"shootingSpeed:"+String(shootingSpeed,1)+","+"jogSpeed:"+String(jogSpeed,1)+","+"deshakeDelay:"+String(deshakeDelay,1)+","+"shootDelay:"+String(shootDelay,1)+","+"overshootDistance:"+String(overshootDistance,1)+",";
    server.send(200, "text/plain", response);
  });
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop(void){
  server.handleClient();
  

  if(current_state==PHOTO_SESSION){
    if((picsTaken<numImages)&&((increment > 0 && currentPos <= startPoint+distance+(increment/2)) || (increment < 0 && currentPos >= startPoint+distance+(increment/2) ))) // +(increment/2) is used as a tolerance for floating point rounding-errors
    {
      Serial.print("Current position: ");
      Serial.print(currentPos);
      Serial.println(" mm");
      // shoot a frame
      delay(deshakeDelay * 1000);
      activateCameraTrigger();
      if(picsTaken<numImages){
        delay(shootDelay * 1000);
        //increment position
        stepper.moveRelativeInMillimeters(increment);
        currentPos += increment;
      }
    }else{
      current_state=JOGGING;
      updateStepperSettings();
    }
  }else if(current_state==JOGGING){
    if(JOGFWDFLAG){
      jog_fwd();
    }else if(JOGBWDFLAG){
      jog_bwd();
    }
  }
}
