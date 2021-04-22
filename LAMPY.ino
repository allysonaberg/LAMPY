
#include <Pixy2.h>
#include <ServoEasing.h>
#include <PIDLoop.h>

//easeTo = blocking, will force wait until complete
//startEaseTo = non-blocking

ServoEasing serMain;
int serMainMax = 90;

ServoEasing serPan;
int serPanMax = 180;

ServoEasing serTilt;
int serTiltMax = 140;
int serTiltMin = 20;

Pixy2 pixy;
PIDLoop panLoop(400, 0, 400, true);
PIDLoop tiltLoop(500, 0, 500, true);

int poser = 0;
int val;
int SPEED = 180;
int SPEEDMEDIUM = 80;
int SPEEDSLOW = 40;
int MOVETHRESHOLD = 0;
int noBlockCounter = 0;
int NOBLOCKTHRESHOLD = 1000;
int switchState = 2;
bool shutdown = false;


int soundPin=A2;
int ledRed=4;
int ledGreen=5;
int ledBlue=6;
int stopButtonPin = 2;
int switchButtonPin = 3;
int threshold=800;
bool ledStatus=false;

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
  //MAX ANGLE: 140, MIN ANGLE: 20
  serTilt.setSpeed(SPEED);
  serTilt.setEasingType(EASE_CUBIC_IN_OUT);
  serTilt.write(90);
  serTilt.attach(10);
  serTilt.easeTo(90);

  pixy.init();
  pixy.changeProg("color_connected_components");  
  pixy.setLamp(0,0);

  pinMode(soundPin, INPUT);
  pinMode(stopButtonPin, INPUT);
  pinMode(switchButtonPin, INPUT);
  
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  mainPosition();
}

void loop() {
  int buttonState = digitalRead(stopButtonPin);
  if (buttonState == HIGH) {
    shutdown = true;
    //shutdown procedure
    endPosition();
    blinkLed(3, 200);
  }

  if (!shutdown) {
    testSound();
    int switchButtonState = digitalRead(switchButtonPin);
    if (switchButtonState == HIGH) {
      if (switchState == 1) {
        Serial.println("SWITCHING MODES: PARSE SERIAL");
        switchState = 2;
        delay(1000);
      } else {
        Serial.println("SWITCHING MODES: PAN TILT");
        switchState = 1;
        delay(1000);
      }
    }

    if (switchState == 1) {
      panTilt();
    } else {
      parseSerial();
    }
  }
}

void panTilt() {
  int32_t panOffset, tiltOffset;
  pixy.ccc.getBlocks();

  testSound();
  
  if (pixy.ccc.numBlocks) {   
             
    panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)pixy.ccc.blocks[0].m_x;
    tiltOffset = (int32_t)pixy.ccc.blocks[0].m_y - (int32_t)pixy.frameHeight/2;  

    // update loops
    if (abs(panOffset) > MOVETHRESHOLD) {
      panLoop.update(panOffset);
      movePan(panLoop.m_command/5.6);
    }
    
    if (abs(tiltOffset) > MOVETHRESHOLD) {
      tiltLoop.update(tiltOffset);
      float moveAmount = tiltLoop.m_command/7.2;

      moveTilt(moveAmount);     
    }    
  } else {
    runRandom();
  }
}

void parseSerial() {
    testSound();
    if (Serial.available()) // if serial value is available
    {
      
      int val = Serial.parseInt();// then read the serial value
      Serial.print(val);
      if (val == 1) {
        nodRoutine(100, 100, 4);
      }

      else if (val == 2) {
        shakeRoutine(140, 100, 4);
      }

      else if (val == 3) {
        upDownRoutine(90, 300, 4);
      }

      else if (val == 4) {
        mainPan();
//        mainTilt();
//        mainPanTilt();
      }

      else if (val == 5) {
        testRange();
      }

      else if (val == 6) {
        cute();
      }
      else if (val == 7) {
        confused();
      } else if (val == 8) {
        happy();
      }
    }
}


void nodRoutine(int maxAngle, int delay_time, int num_nods) {
  serPan.easeTo(90);
  for (int i = 0; i < num_nods; i++) {
    moveTilt(maxAngle);
    moveTilt(60);
    delay(delay_time);
  }

}

void shakeRoutine(int maxAngle, int delay_time, int num_nods) {
  moveTilt(90);
  for (int i = 0; i < num_nods; i++) {
    movePan(maxAngle);
    movePan(0);
    delay(delay_time);
  }
}

void upDownRoutine(int maxAngle, int delay_time, int num_nods) {
  moveTilt(90);
  for (int i = 0; i < num_nods; i++) {
    moveMain(maxAngle);
    moveMain(0);
    delay(delay_time);
  }
}

void testSound() {
  int soundsens=analogRead(soundPin);
//  Serial.print(soundsens);
//  Serial.print("/n");
  if (soundsens<=threshold) {
    if(ledStatus==false){
      ledStatus=true;
      switchLed(true);
      delay(250);
    }
    else {
      ledStatus=false;
      switchLed(false);
      delay(250);
    }
  }
}

void moveMain(int angle) {
  if (angle <= serMainMax) {
    serMain.easeTo(angle, SPEEDSLOW);
  } else {
    Serial.print("EXCEEDED MAX ANGLE");
  }
}

void movePan(int angle) {
  if (angle <= serPanMax) {
    serPan.easeTo(angle, SPEED);
  } else {
    Serial.print("EXCEEDED MAX ANGLE");
  }
}

void moveTilt(int angle) {
   if (angle <= serTiltMax && angle >= serTiltMin) {
    serTilt.easeTo(angle, SPEED);
    }  else {
    Serial.print("EXCEEDED MAX ANGLE");
  }
}

void mainPan() {
  serMain.startEaseTo(80, SPEEDSLOW, false);
  serTilt.startEaseTo(90, SPEEDMEDIUM, false);
  serPan.startEaseTo(180, SPEEDMEDIUM, false);
  do {
    delayAndUpdateAndWaitForAllServosToStop(100);
  } while (serMain.isMoving() || serPan.isMoving() || serTilt.isMoving());

  serMain.startEaseTo(0, SPEEDSLOW, false);
  serTilt.startEaseTo(120, SPEEDMEDIUM, false);
  serPan.startEaseTo(0, SPEEDMEDIUM, false);
  do {
    delayAndUpdateAndWaitForAllServosToStop(100);
  } while (serMain.isMoving() || serPan.isMoving() || serTilt.isMoving());
}

void mainTilt() {
  serTilt.startEaseTo(90, SPEEDSLOW, false);
  serPan.startEaseTo(180, SPEEDMEDIUM, false);
  serMain.startEaseTo(0, SPEEDSLOW, false);
  do {
    delayAndUpdateAndWaitForAllServosToStop(100);
  } while (serMain.isMoving() || serPan.isMoving() || serTilt.isMoving()); 
}

void mainPanTilt() {
  serTilt.startEaseTo(180, SPEED, false);
  serPan.startEaseTo(0, SPEED, false);
  serMain.startEaseTo(800, SPEEDSLOW, false);
  do {
    delayAndUpdateAndWaitForAllServosToStop(100);
  } while (serMain.isMoving() || serPan.isMoving() || serTilt.isMoving());
  
}

void testRange() {
  Serial.print("MAIN 0");
  serMain.easeTo(0, SPEEDSLOW);
  delay(1000);

  Serial.print("MAIN 90");
  serMain.easeTo(90, SPEEDSLOW);
  delay(1000);

  Serial.print("TILT 20");
  serTilt.easeTo(20, SPEED);
  delay(1000);
  Serial.print("TILT 140");
  serTilt.easeTo(140, SPEED);
  delay(1000);

  Serial.print("PAN 0");
  serPan.easeTo(0, SPEED);
  delay(1000);
  Serial.print("PAN 180");
  serPan.easeTo(180, SPEED);
  delay(1000);

  serMain.easeTo(0, SPEEDSLOW);
  serTilt.easeTo(0, SPEED);
  serPan.easeTo(0, SPEED);

  mainPosition();
}

void cute() {
  serMain.startEaseTo(80, SPEEDSLOW, true);
  serTilt.startEaseTo(140, SPEEDMEDIUM, true);
  blinkLed(2, 300);
  serMain.startEaseTo(0, SPEEDSLOW, true);
  delay(1000);
  serPan.startEaseTo(0, SPEED, true);
  delay(1000);
  serPan.startEaseTo(180, SPEED, true);
  delay(1500);
  mainPosition();
  blinkLed(3, 300);
}

void confused() {
  serMain.startEaseTo(20, SPEEDSLOW, true);
  serTilt.startEaseTo(140, SPEEDMEDIUM, true);
  delay(800);
  serPan.startEaseTo(0, SPEED, true);
  delay(1000);
  serPan.startEaseTo(180, SPEED, true);
  delay(1200);
  serPan.startEaseTo(0, SPEED, true);
  serTilt.startEaseTo(0, SPEED, true);
  serMain.startEaseTo(45, SPEEDSLOW, true);
  delay(800);
  serPan.startEaseTo(180, SPEED, true);
  serMain.startEaseTo(45, SPEED, true);
  delay(1200);
  blinkLed(3, 300);

  mainPosition();
}

void happy() {
  serMain.startEaseTo(30, SPEEDSLOW, true);
  serTilt.startEaseTo(30, SPEEDMEDIUM, true);
  blinkLed(2, 300);
  delay(500);
  serPan.startEaseTo(0, SPEED, true);
  delay(500);
  serPan.startEaseTo(180, SPEED, true);
  blinkLed(2, 300);
  serMain.startEaseTo(90, SPEEDSLOW, true);
  delay(800);
  serTilt.startEaseTo(30, SPEEDMEDIUM, true);
  delay(800);  
  serTilt.startEaseTo(130, SPEEDMEDIUM, true);
  delay(800); 
  serTilt.startEaseTo(30, SPEEDMEDIUM, true);
  delay(800);
  serTilt.startEaseTo(130, SPEEDMEDIUM, true);
  delay(1000);  
  
  mainPosition();
  blinkLed(3, 300);
}

void mainPosition() {
  serMain.startEaseTo(10);
  serPan.startEaseTo(90);
  serTilt.startEaseTo(90);
  do {
    delayAndUpdateAndWaitForAllServosToStop(0);
  } while (serPan.isMoving() || serTilt.isMoving() || serMain.isMoving());
}

void endPosition() {
  serPan.startEaseTo(90);
  serMain.startEaseTo(0);
  serTilt.startEaseTo(140);
  do {
    delayAndUpdateAndWaitForAllServosToStop(0);
  } while (serPan.isMoving() || serTilt.isMoving() || serMain.isMoving());
}

void blinkLed(int num_blinks, int delay_time) {
  for (int i = 0; i < num_blinks; i++) {
    switchLed(true);
    delay(delay_time);
    switchLed(false);
    delay(delay_time);
  }
}
void switchLed(bool isOn) {
  if (isOn) {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledBlue, HIGH);
  } else {
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledBlue, LOW);
  }
}

void runRandom() {
  testSound();
    if (noBlockCounter >= NOBLOCKTHRESHOLD) {
      noBlockCounter = 0;
      int randNumber = random(2);

      if (randNumber == 0) {
        cute();
      } else if (randNumber == 1) {
        confused();
      } else {
        happy();
      }
    } else {
      noBlockCounter++;
    }
}
