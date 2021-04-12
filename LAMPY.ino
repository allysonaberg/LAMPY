
#include <Pixy2.h>
#include <ServoEasing.h>
#include <PIDLoop.h>
//

ServoEasing serMain;
ServoEasing serPan;
ServoEasing serTilt;

Pixy2 pixy;
PIDLoop panLoop(450, 0, 700, true);
PIDLoop tiltLoop(450, 0, 700, true);

int poser = 0;
int val;
int SPEED = 200;
int SPEEDSLOW = 50;
int MOVETHRESHOLD = 20;
bool shutdown = false;

void setup() {

  //MAIN SERVO, PIN 8
  //MAX ANGLE: 90 (70 for better performance)
  Serial.begin(115200);
  Serial.print("Starting...\n");
  serMain.setSpeed(SPEEDSLOW);
  serMain.setEasingType(EASE_CUBIC_IN_OUT);
  serMain.write(0);
  serMain.attach(8);

  //PAN, PIN 9
  //MAX ANGLE: 180
  serPan.setSpeed(SPEED);
  serPan.setEasingType(EASE_CUBIC_IN_OUT);
  serPan.write(0);
  serPan.attach(9);

  //TILT, PIN 10
  //MAX ANGLE: 140?? TEST
  serTilt.setSpeed(SPEED);
  serTilt.setEasingType(EASE_CUBIC_IN_OUT);
  serTilt.write(0);
  serTilt.attach(10);

  serTilt.easeTo(0);
  serPan.easeTo(90);
 
  
  pixy.init();
  pixy.changeProg("video");  
}

void loop() {
  panTilt();
//  parseSerial();
//  runRoutine();
}

void panTilt() {
  int32_t panOffset, tiltOffset;
  pixy.ccc.getBlocks();
  
  if (pixy.ccc.numBlocks) {            
    panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)pixy.ccc.blocks[0].m_x;
    tiltOffset = (int32_t)pixy.ccc.blocks[0].m_y - (int32_t)pixy.frameHeight/2;  

    Serial.println(tiltOffset);

    // update loops
    if (abs(panOffset) > MOVETHRESHOLD) {
      panLoop.update(panOffset);
      serPan.easeTo(panLoop.m_command/5.6, SPEED);

    }
    
    if (abs(tiltOffset) > MOVETHRESHOLD) {
      tiltLoop.update(tiltOffset);
      serTilt.easeTo(tiltLoop.m_command/7.2, SPEED);     
    }

    
  }
}

void parseSerial() {
    while (!shutdown) {
    if (Serial.available()) // if serial value is available
    {
      
      int val = Serial.parseInt();// then read the serial value
      Serial.print(val);
      if (val >= 0 && val <= 80) {
        serMain.startEaseTo(val, SPEEDSLOW, false);
        serPan.startEaseTo(val, SPEED, false);
        serTilt.startEaseTo(val, SPEED, false);
        do {
          delayAndUpdateAndWaitForAllServosToStop(100);
        } while (serMain.isMoving() || serPan.isMoving() || serTilt.isMoving());
      } 

      if (val == -1) {
        Serial.print("shutdown");
        shutdown = true;
        serMain.startEaseTo(0, SPEEDSLOW, false);
        serPan.startEaseTo(0, SPEED, false);
        serTilt.startEaseTo(0, SPEED, false);
        do {
          delayAndUpdateAndWaitForAllServosToStop(100);
        } while (serMain.isMoving() || serPan.isMoving() || serTilt.isMoving());
      }
    }
  }
}



void runRoutine() {
  //easeTo = blocking, will force wait until complete
  //startEaseTo = non-blocking

  while (!shutdown) {
      Serial.print("ease to 180/140");
      serPan.startEaseTo(180, SPEED);
      serTilt.startEaseTo(140, SPEED);
      Serial.print("ease to 0");
      serPan.startEaseTo(0, SPEED);
      serTilt.startEaseTo(0, SPEED);

      do {
          delayAndUpdateAndWaitForAllServosToStop(100);
        } while (serPan.isMoving() || serTilt.isMoving());

    if (Serial.available())
    {
      int val = Serial.parseInt();
      if (val == -1) {
        Serial.print("shutdown");
        shutdown = true;
        serPan.startEaseTo(0, SPEED, false);
        serTilt.startEaseTo(0, SPEED, false);
        do {
          delayAndUpdateAndWaitForAllServosToStop(100);
        } while (serPan.isMoving() || serTilt.isMoving());
      }
    }
  }
}
