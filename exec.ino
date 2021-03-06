/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  http://code.google.com/p/polargraph/

Exec.

This is one of the core files for the polargraph server program.  
Purposes are getting a little more blurred here.  This file contains
the basic decision tree that branches based on command.

It has a set of the most general-purpose drawing commands, but only
methods that are directly called - none of the geometry or conversion
routines are here.

*/
/**  This method looks only for the basic command set
*/
boolean exec_executeBasicCommand(String inCmd, String inParam1, String inParam2, String inParam3, String inParam4, String inNoOfParams)
{
  #ifdef DEBUG
  Serial.print("Command: ");
  Serial.println(inCmd);
  #endif
  boolean executed = true;
  if (inCmd.startsWith(CMD_CHANGELENGTH))
    exec_changeLength();
  else if (inCmd.startsWith(CMD_CHANGELENGTHDIRECT))
    exec_drawStraightToPoint();
  else if (inCmd.startsWith(CMD_SETMOTORSPEED))
    exec_setMotorSpeed();
  else if (inCmd.startsWith(CMD_SETMOTORACCEL))
    exec_setMotorAcceleration();
  else if (inCmd.startsWith(CMD_SETPOSITION))
    exec_setPosition();
  else if (inCmd.startsWith(CMD_PENDOWN))
    penlift_penDown();
  else if (inCmd.startsWith(CMD_PENUP))
    penlift_penUp();
  else if (inCmd.startsWith(CMD_SETMACHINESIZE))
    exec_setMachineSizeFromCommand();
  else if (inCmd.startsWith(CMD_SETMACHINEMMPERREV))
    exec_setMachineMmPerRevFromCommand();
  else if (inCmd.startsWith(CMD_SETMACHINESTEPSPERREV))
    exec_setMachineStepsPerRevFromCommand();
  else if (inCmd.startsWith(CMD_SETPENLIFTRANGE))
    exec_setPenLiftRange();
  else if (inCmd.startsWith(CMD_GETMACHINEDETAILS))
    exec_reportMachineSpec();
  else if (inCmd.startsWith(CMD_RESETEEPROM))
    eeprom_resetEeprom();
  else if (inCmd.startsWith(CMD_ACTIVATE_BUTTON))
    exec_changeReadingButton(true);
  else if (inCmd.startsWith(CMD_DEACTIVATE_BUTTON))
    exec_changeReadingButton(false);
  else if (inCmd.startsWith(CMD_RESET_MACHINE)) {
    CPU_RESTART
  }
  else
    executed = false;

  return executed;
}

void exec_changeReadingButton(boolean reading)
{
  if (reading) {
    digitalWrite(INDICATOR_LED, HIGH);
    readingButton = true;
  }
  else {
    digitalWrite(INDICATOR_LED, LOW);
    readingButton = false;
  }
}

void exec_reportMachineSpec()
{
  eeprom_dumpEeprom();

  Serial.print(F("PGSIZE,"));
  Serial.print(int(machineWidth));
  Serial.print(COMMA);
  Serial.print(int(machineHeight));
  Serial.println(CMD_END);

  Serial.print(F("PGMMPERREV,"));
  Serial.print(mmPerRev);
  Serial.println(CMD_END);

  Serial.print(F("PGSTEPSPERREV,"));
  Serial.print(stepsPerRev);
  Serial.println(CMD_END);
  
  Serial.print(F("PGSTEPMULTIPLIER,"));
  Serial.print(stepMultiplier);
  Serial.println(CMD_END);
  
  Serial.print(F("PGLIFT,"));
  Serial.print(downPosition);
  Serial.print(COMMA);
  Serial.print(upPosition);
  Serial.println(CMD_END);

  Serial.print(F("PGSPEED,"));
  Serial.print(maxSpeed);
  Serial.print(COMMA);
  Serial.print(accel);
  Serial.println(CMD_END);

}

void exec_setMachineSizeFromCommand()
{
  int width = asInt(inParam1);
  int height = asInt(inParam2);

  // load to get current settings
  int currentValue = width;
  EEPROM_readAnything(EEPROM_MACHINE_WIDTH, currentValue);  
  if (currentValue != width)
    if (width > 10)
    {
      EEPROM_writeAnything(EEPROM_MACHINE_WIDTH, width);
    }
  
  EEPROM_readAnything(EEPROM_MACHINE_HEIGHT, currentValue);
  if (currentValue != height)
    if (height > 10)
    {
      EEPROM_writeAnything(EEPROM_MACHINE_HEIGHT, height);
    }

  // reload 
  eeprom_loadMachineSize();
}


//void exec_setMachineNameFromCommand()
//{
//  String name = inParam1;
//  if (name != DEFAULT_MACHINE_NAME)
//  {
//    for (int i = 0; i < 8; i++)
//    {
//      EEPROM.write(EEPROM_MACHINE_NAME+i, name[i]);
//    }
//  }
//  eeprom_loadMachineSpecFromEeprom();
//}

void exec_setMachineMmPerRevFromCommand()
{
  float mmPerRev = asFloat(inParam1);
  EEPROM_writeAnything(EEPROM_MACHINE_MM_PER_REV, mmPerRev);
  eeprom_loadMachineSpecFromEeprom();
}
void exec_setMachineStepsPerRevFromCommand()
{
  int stepsPerRev = asInt(inParam1);
  EEPROM_writeAnything(EEPROM_MACHINE_STEPS_PER_REV, stepsPerRev);
  eeprom_loadMachineSpecFromEeprom();
}

void exec_setPenLiftRange()
{
  int down = asInt(inParam1);
  int up = asInt(inParam2);
  
  Serial.print(F("Down: "));
  Serial.println(down);
  Serial.print(F("Up: "));
  Serial.println(up);
  
  if (inNoOfParams == 4) 
  {
    // 4 params (C45,<downpos>,<uppos>,1,END) means save values to EEPROM
    EEPROM_writeAnything(EEPROM_PENLIFT_DOWN, down);
    EEPROM_writeAnything(EEPROM_PENLIFT_UP, up);
//    eeprom_loadPenLiftRange();
  }
  else if (inNoOfParams == 3)
  {
    // 3 params (C45,<downpos>,<uppos>,END) means just do a range test
    penlift_movePen(up, down, penLiftSpeed);
    delay(200);
    penlift_movePen(down, up, penLiftSpeed);
    delay(200);
    penlift_movePen(up, down, penLiftSpeed);
    delay(200);
    penlift_movePen(down, up, penLiftSpeed);
    delay(200);
  }
}

void exec_setMotorSpeed()
{
  maxSpeed = asFloat(inParam1);
  motorA.setMaxSpeed(maxSpeed);
  motorB.setMaxSpeed(maxSpeed);
  Serial.print(F("New max speed: "));
  Serial.println(maxSpeed);
}

void exec_setMotorAcceleration()
{
  accel = asFloat(inParam1);
  motorA.setAcceleration(accel);
  motorB.setAcceleration(accel);
  Serial.print(F("New acceleration: "));
  Serial.println(accel);
}


void exec_setPosition()
{
  float x = asFloat(inParam1);
  float y = asFloat(inParam2);
  
  float a = getMachineA(x, y);
  float b = getMachineB(machineWidth, x, y);

  motorA.writeEnc(mmToEncoderSteps(a));
  motorA.synchroniseMotorWithEncoder();
  motorB.writeEnc(mmToEncoderSteps(b));
  motorB.synchroniseMotorWithEncoder();
  
  isCalibrated = true;
}

void exec_changeLength()
{
  motors_engage();
  float x = asFloat(inParam1);
  float y = asFloat(inParam2);
  
  float a = getMachineA(x, y);
  float b = getMachineB(machineWidth, x, y);

  Serial.print(F("Cartesian mm: "));
  Serial.print(x);
  Serial.print(COMMA);
  Serial.println(y);
  Serial.print(F("Native mm: "));
  Serial.print(a);
  Serial.print(COMMA);
  Serial.println(b);
  Serial.print(F("Native mSteps: "));
  Serial.print(mmToMotorSteps(a));
  Serial.print(COMMA);
  Serial.println(mmToMotorSteps(b));
  
  exec_changeLength(mmToMotorSteps(a), mmToMotorSteps(b));
}

void exec_changeLength(float a, float b) {
  if (!powerOn) motors_engage();
  int speedA = motorA.speed();
  int speedB = motorB.speed();
  motorA.moveTo(a);
  motorB.moveTo(b);
  while (motorA.distanceToGo() != 0 || motorB.distanceToGo() != 0)
  {
    motorA.run();
    motorB.run();
  }
}

void exec_changeLengthAtSpeed(float a, float b, long maxSpeed) {
  if (!powerOn) motors_engage();
  motorA.moveTo(a);
  motorB.moveTo(b);
  
  float speed = abs(maxSpeed);
  float aDist = motorA.distanceToGo();
  float bDist = motorB.distanceToGo();
  float distDiff = abs(aDist) - abs(bDist);
  float aSpeed = speed;
  float bSpeed = speed;
  
#ifdef DEBUG_DISTANCE_TO_GO
  Serial.print("aSpeed: ");
  Serial.print(aSpeed);
  Serial.print(", bSpeed: ");
  Serial.println(bSpeed);
  Serial.print("Distancetogo: ");
  Serial.print(aDist);
  Serial.print(",");
  Serial.print(bDist);
  Serial.print(", diff: ");
  Serial.println(distDiff);
#endif
  
  if (distDiff > 0) {
    // a is longest move
    aSpeed = speed;
    bSpeed = speed / (abs(aDist) / abs(bDist));
  }
  else if (distDiff < 0) {
    aSpeed = speed / (abs(bDist) / abs(aDist));
    bSpeed = speed;
  }

#ifdef DEBUG_DISTANCE_TO_GO
  Serial.print("aSpeed: ");
  Serial.print(aSpeed);
  Serial.print(", bSpeed: ");
  Serial.println(bSpeed);
  Serial.print("Distancetogo: ");
  Serial.print(aDist);
  Serial.print(",");
  Serial.print(bDist);
  Serial.print(", diff: ");
  Serial.println(distDiff);
#endif

  while (aDist != 0 || bDist != 0)
  {
#ifdef DEBUG_DISTANCE_TO_GO
    Serial.print("Distancetogo: ");
    Serial.print(aDist);
    Serial.print(",");
    Serial.println(bDist);
#endif

    if (aDist < 0) motorA.setSpeed(-aSpeed);
    else motorA.setSpeed(aSpeed);
    if (bDist < 0) motorB.setSpeed(-bSpeed);
    else motorB.setSpeed(bSpeed);
    
    if (aDist != 0) motorA.runSpeed();
    if (bDist != 0) motorB.runSpeed();

    aDist = motorA.distanceToGo();
    bDist = motorB.distanceToGo();
  }
  
//  Serial.print("Deviation A, B: ");
//  Serial.print(motorA.computeDeviation());
//  Serial.print(", ");
//  Serial.println(motorB.computeDeviation());
  
  motorA.correctDeviation();
  motorB.correctDeviation();
}

void exec_changeLengthCartesianMm(float x, float y) {
  float pA = getMachineA(x, y);
  float pB = getMachineB(machineWidth, x, y);
  pA = mmToMotorSteps(pA);
  pB = mmToMotorSteps(pB);
  
  if (usingAcceleration)
    exec_changeLength(pA, pB);
  else
    exec_changeLengthAtSpeed(pA, pB, motorA.speed());
}

/**
This moves the gondola in a straight line between p1 and p2.  
Both input coordinates are cartesian mm.  

The fidelity of the line is controlled by maxLength - this is the longest size a line segment is 
allowed to be.  1 is finest, slowest.  Use higher values for faster, wobblier.
*/
void exec_drawStraightToPoint()
{
  // ok, we're going to plot some dots between p1 and p2.  Using maths. I know! Brave new world etc.
  
  // First, convert these values to cartesian coordinates
  // We're going to figure out how many segments the line
  // needs chopping into.
  maxSegmentLength = asInt(inParam3);
  
  float mmc2x = asFloat(inParam1);
  float mmc2y = asFloat(inParam2);
  float c2x = mmToMotorSteps(mmc2x);
  float c2y = mmToMotorSteps(mmc2y);
  
  float mmn1a = encoderStepsToMm(motorA.readEnc());
  float mmn1b = encoderStepsToMm(motorB.readEnc());
  float mmc1x = getCartesianX(machineWidth, mmn1a, mmn1b);
  float mmc1y = getCartesianY(mmc1x, mmn1a);
  float c1x = mmToMotorSteps(mmc1x);
  float c1y = mmToMotorSteps(mmc1y);
  
  float machineWidthSteps = mmToMotorSteps(machineWidth);
  float machineHeightSteps = mmToMotorSteps(machineHeight);
  float margin = 20.0;

#ifdef DEBUG
  Serial.print("From coords (mm-cart): ");
  Serial.print(mmc1x);
  Serial.print(",");
  Serial.println(mmc1y);
  Serial.print("From coords (mm-native): ");
  Serial.print(mmn1a);
  Serial.print(",");
  Serial.println(mmn1b);
  Serial.print("To coords (mm-cart): ");
  Serial.print(mmc2x);
  Serial.print(",");
  Serial.println(mmc2y);

  Serial.print("From coords (steps): ");
  Serial.print(c1x);
  Serial.print(",");
  Serial.println(c1y);
  Serial.print("To coords (steps): ");
  Serial.print(c2x);
  Serial.print(",");
  Serial.println(c2y);
#endif

  // test to see if it's on the page
  // AND ALSO TO see if the current position is on the page.
  // Remember, the native system can easily specify points that can't exist,
  // particularly up at the top.
  if (mmc2x > margin 
    && mmc2x < machineWidth-margin 
    && mmc2y > margin 
    && mmc2y < machineHeight-margin
    && mmc1x > margin 
    && mmc1x < machineWidth-margin 
    && mmc1y > margin 
    && mmc1y < machineHeight-margin 
    )
    {
    reportingPosition = false;
    float deltaX = mmc2x-mmc1x;    // distance each must move (signed)
    float deltaY = mmc2y-mmc1y;
//    float totalDistance = sqrt(sq(deltaX) + sq(deltaY));

    long linesegs = 1;            // assume at least 1 line segment will get us there.
    if (maxSegmentLength < 2) maxSegmentLength = 10;
    if (abs(deltaX) > abs(deltaY))
    {
      #ifdef DEBUG
      Serial.println("x is biggger than y");
      #endif
      // slope <=1 case    
      while ((abs(deltaX)/(float)linesegs) > maxSegmentLength)
      {
        linesegs++;
      }
    }
    else
    {

      // slope >1 case
      
      while ((abs(deltaY)/(float)linesegs) > maxSegmentLength)
      {
        linesegs++;
      }
    }
    

#ifdef DEBUG
    Serial.print("Line segs: ");
    Serial.println(linesegs);
#endif
    // reduce delta to one line segments' worth.
    deltaX = deltaX/(float)linesegs;
    deltaY = deltaY/(float)linesegs;
  
    // render the line in N shorter segments
    long runSpeed = 0;

    usingAcceleration = false;
    while (linesegs > 0)
    {
#ifdef DEBUG
      Serial.print("Line segment: " );
      Serial.print(linesegs);
      Serial.print(". Segment is ");
      Serial.print(deltaX);
      Serial.print(",");
      Serial.print(deltaY);
      Serial.println(" steps.");
      Serial.print("So, moving from ");
      Serial.print(mmc1x);
      Serial.print(",");
      Serial.print(mmc1y);
      Serial.print("mm to ");
#endif
      // compute next new location
      mmc1x = mmc1x + deltaX;
      mmc1y = mmc1y + deltaY;
#ifdef DEBUG
      Serial.print(mmc1x);
      Serial.print(",");
      Serial.print(mmc1y);
      Serial.print("mm.");
#endif  
      // do the move
      runSpeed = exec_desiredSpeed(linesegs, runSpeed, accel*10);
#ifdef DEBUG
      Serial.print("Setting speed:");
      Serial.println(runSpeed);
#endif
      motorA.setSpeed(runSpeed);
      motorB.setSpeed(runSpeed);
      exec_changeLengthCartesianMm(mmc1x, mmc1y);
  
      // one line less to do!
      linesegs--;
    }
    // reset back to "normal" operation
    reportingPosition = true;
    usingAcceleration = true;
//    reportPosition();
  }
  else
  {
    Serial.println("Line is not on the page. Skipping it.");
  }
}

/*
This is a method pinched from AccelStepper (older version), thanks Mike McCauley.
*/
float exec_desiredSpeed(long distanceTo, float currentSpeed, float acceleration)
{
    float requiredSpeed;

    if (distanceTo == 0)
	return 0.0f; // We're there

    // sqrSpeed is the signed square of currentSpeed.
    float sqrSpeed = sq(currentSpeed);
    if (currentSpeed < 0.0)
      sqrSpeed = -sqrSpeed;
      
    float twoa = 2.0f * acceleration; // 2ag
    
    // if v^^2/2as is the the left of target, we will arrive at 0 speed too far -ve, need to accelerate clockwise
    if ((sqrSpeed / twoa) < distanceTo)
    {
	// Accelerate clockwise
	// Need to accelerate in clockwise direction
	if (currentSpeed == 0.0f)
	    requiredSpeed = sqrt(twoa);
	else
	    requiredSpeed = currentSpeed + fabs(acceleration / currentSpeed);

	if (requiredSpeed > maxSpeed)
	    requiredSpeed = maxSpeed;
    }
    else
    {
	// Decelerate clockwise, accelerate anticlockwise
	// Need to accelerate in clockwise direction
	if (currentSpeed == 0.0f)
	    requiredSpeed = -sqrt(twoa);
	else
	    requiredSpeed = currentSpeed - fabs(acceleration / currentSpeed);
	if (requiredSpeed < -maxSpeed)
	    requiredSpeed = -maxSpeed;
    }
#ifdef DEBUG    
    Serial.println(requiredSpeed);
#endif
    return requiredSpeed;
}

