#include <Encoder.h>
#include <AccelStepperEncoder.h>

// encoders, left and right
Encoder encA(18, 19);
Encoder encB(6, 5);

// AccelStepperEncoder objects
AccelStepperEncoder motorA(AccelStepperEncoder::DRIVER, 14, 15, 1, 1, false);
AccelStepperEncoder motorB(AccelStepperEncoder::DRIVER, 1, 2, 1, 1, false);

// number of motor steps per encoder step
float motorToEncoderRatio = 1.51;

// Endstop pins
int leftEndStopPin = 21;
int rightEndStopPin = 22;

// variable that controls whether the motors step or not.
volatile boolean runningMotors = true;

// definition of the motor and drive train
float mmPerRev = 97.0;
float stepsPerRev = 200.0 * 8.0;
float encStepsPerRev = stepsPerRev / motorToEncoderRatio;

float stepsPerMm = stepsPerRev / mmPerRev; // 16.49
float mmPerStep = mmPerRev / stepsPerRev;  // 0.606

float encStepsPerMm = encStepsPerRev / mmPerRev;
float mmPerEncStep = mmPerRev / encStepsPerRev;

// machine size
float machineWidth = 308.0;
float machineHeight = 450.0;

// Endstop positions in mm. This is really a description of the distance
// from the magnet to the pen tip.
long leftEndStopOffset = 85.0;
long rightEndStopOffset = 97.0;

// interval timer that will run the stepper motors...
IntervalTimer motorTimer;
// ... and the function that get's called by it
void runMotors(void) {
  if (runningMotors) {
    motorB.run();
    motorA.run();
  }
}

// Equilibrium position - where the pen was when it was turned on.
float equilibriumA = 0;
float equilibriumB = 0;

float mmToMotorSteps(float mm) {
  return mm * stepsPerMm;
}
float mmToEncoderSteps(float mm) {
  return mm * encStepsPerMm;
}
float motorStepsToMm(float steps) {
  return steps * mmPerStep;
}
float encoderStepsToMm(float steps) {
  return steps * mmPerEncStep;
}

float encoderToMotorSteps(float encSteps) {
  return encSteps * motorToEncoderRatio;
}
float motorToEncoderSteps(float motorSteps) {
  return motorSteps / motorToEncoderRatio;
}

void setup() {
  Serial.begin(9600);
  delay(3000); 
  Serial.println("Polargraph Pro");
  pinMode(rightEndStopPin, INPUT); 
  pinMode(leftEndStopPin, INPUT); 

  motorA.setPinsInverted(false, false, true);
  motorA.setEnablePin(17);
  motorA.addEncoder(&encA, motorToEncoderRatio);
  motorA.setAcceleration(3000);
  motorA.setMaxSpeed(1000);
  
  motorB.setPinsInverted(true, false, true);
  motorB.setEnablePin(4);
  motorB.addEncoder(&encB, motorToEncoderRatio);
  motorB.setAcceleration(3000);
  motorB.setMaxSpeed(1000);

  motorTimer.begin(runMotors, 1000);
  
  calibrateHome();
}

/*
Calibrate winds one cord in, and then the other, until endstop switches are 
triggered by magnets on the cords.

The distance from the pen tip to the magnets is fixed, and known, measured 
manually during the fitting of the magnets.
*/
void calibrateHome() {
  motorA.enableOutputs();
  motorB.enableOutputs();
  motorA.setAcceleration(10000);
  motorB.setAcceleration(10000);
  while (digitalRead(leftEndStopPin) == HIGH) {
    motorA.move(-10);
    motorB.move(+12);
  }
  delay(500);
  float leftWoundIn = abs(motorA.readEnc());
  float leftDistanceInEncoderSteps = mmToEncoderSteps(leftEndStopOffset);
  motorA.writeEnc(leftDistanceInEncoderSteps);
  motorA.synchroniseMotorWithEncoder();
  equilibriumA = encoderStepsToMm(leftWoundIn) - leftEndStopOffset;
 
  
  
  while (digitalRead(rightEndStopPin) == HIGH) {
    motorB.move(-10);
    motorA.move(+12);
  }
  delay(500);
  float rightWoundIn = abs(motorA.readEnc());
  float rightDistanceInEncoderSteps = mmToEncoderSteps(rightEndStopOffset);
  motorB.writeEnc(rightDistanceInEncoderSteps);
  motorB.synchroniseMotorWithEncoder();
  equilibriumB = encoderStepsToMm(rightWoundIn) - rightEndStopOffset;
  

  motorA.setAcceleration(3000);
  motorB.setAcceleration(3000);
  motorA.enableOutputs();
  motorB.enableOutputs();
  motorB.moveTo(mmToMotorSteps(equilibriumA));
  motorA.moveTo(mmToMotorSteps(equilibriumB));

  motorB.disableOutputs();
  motorA.disableOutputs();
}

float getMachineA(float cX, float cY)
{
  float a = sqrt(sq(cX)+sq(cY));
  return a;
}
float getMachineB(float cX, float cY)
{
  float b = sqrt(sq((machineWidth)-cX)+sq(cY));
  return b;
}

float getCartesianX(float aPos, float bPos)
{
  float calcX = (sq((float)machineWidth) - sq((float)bPos) + sq((float)aPos)) / ((float)machineWidth*2.0);
  return calcX;  
}
float getCartesianY(float cX, float aPos) 
{
  float calcY = sqrt(sq(aPos)-sq(cX));
  return calcY;
}

void loop() {
  
}

