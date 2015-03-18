void motors_release() {
  motorA.disableOutputs();
  motorB.disableOutputs();
  powerOn = false;
}

void motors_engage() {
  motorA.enableOutputs();
  motorB.enableOutputs();
  powerOn = true;
}

/*
Calibrate winds one cord in, and then the other, until endstop switches are 
triggered by magnets on the cords.

The distance from the pen tip to the magnets is fixed, and known, measured 
manually during the fitting of the magnets.
*/
void motors_calibrateHome() {
  Serial.println("Calibrating.");
  executing = true;
  motorTimer.begin(runMotors, motorRunRate);
  
  motors_release();
  delay(3000);
  motors_engage();
  motorA.setAcceleration(10000);
  motorB.setAcceleration(10000);
  while (digitalRead(leftEndStopPin) == HIGH) {
    motorA.move(-20);
    motorB.move(+24);
  }
  delay(500);
  float leftWoundIn = abs(motorA.readEnc());
  float leftDistanceInEncoderSteps = mmToEncoderSteps(leftEndStopOffset);
  motorA.writeEnc(leftDistanceInEncoderSteps);
  motorA.synchroniseMotorWithEncoder();
  equilibriumA = encoderStepsToMm(leftWoundIn) + leftEndStopOffset;

#ifdef DEBUG  
  Serial.println("Left Homing: ");
  Serial.print("Wound in (eSteps): ");Serial.print(leftWoundIn);
  Serial.print(", and in mm: ");Serial.println(encoderStepsToMm(leftWoundIn));
  Serial.print("Distance from pentip to bead in eSteps: ");Serial.print(leftDistanceInEncoderSteps);
  Serial.print(", and in mm: ");Serial.println(leftEndStopOffset);
  Serial.print("Equilibrium in eSteps: ");Serial.print(encoderStepsToMm(equilibriumA));
  Serial.print(", and in mm: ");Serial.print(equilibriumA);
#endif
  
  
  while (digitalRead(rightEndStopPin) == HIGH) {
    motorB.move(-20);
    motorA.move(+24);
  }
  delay(500);
  float rightWoundIn = abs(motorB.readEnc());
  float rightDistanceInEncoderSteps = mmToEncoderSteps(rightEndStopOffset);
  motorB.writeEnc(rightDistanceInEncoderSteps);
  motorB.synchroniseMotorWithEncoder();
  equilibriumB = encoderStepsToMm(rightWoundIn) + rightEndStopOffset;

#ifdef DEBUG  
  Serial.println("Right Homing: ");
  Serial.print("Wound in (eSteps): ");Serial.print(rightWoundIn);
  Serial.print(", and in mm: ");Serial.println(encoderStepsToMm(rightWoundIn));
  Serial.print("Distance from pentip to bead in eSteps: ");Serial.print(rightDistanceInEncoderSteps);
  Serial.print(", and in mm: ");Serial.println(rightEndStopOffset);
  Serial.print("Equilibrium in eSteps: ");Serial.print(encoderStepsToMm(equilibriumB));
  Serial.print(", and in mm: ");Serial.println(equilibriumB);
#endif
  
  motorA.synchroniseMotorWithEncoder();
  motorB.synchroniseMotorWithEncoder();
  
  motorTimer.end();
  motorA.setAcceleration(3000);
  motorB.setAcceleration(3000);
  
  exec_changeLength(mmToMotorSteps(equilibriumA), mmToMotorSteps(equilibriumB));
 
  isCalibrated = true;
  executing = false;
//  motors_release();
}
