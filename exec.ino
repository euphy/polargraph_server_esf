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
//  else if (com.startsWith(CMD_SETMACHINENAME))
//    exec_setMachineNameFromCommand();
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
  else
    executed = false;

  return executed;
}

void exec_reportMachineSpec()
{
  eeprom_dumpEeprom();

  Serial.print(F("PGSIZE,"));
  Serial.print(machineWidth);
  Serial.print(COMMA);
  Serial.print(machineHeight);
  Serial.println(CMD_END);

  Serial.print(F("PGMMPERREV,"));
  Serial.print(mmPerRev);
  Serial.println(CMD_END);

  Serial.print(F("PGSTEPSPERREV,"));
  Serial.print(stepsPerRev);
  Serial.println(CMD_END);
  
//  Serial.print(F("PGLIFT,"));
//  Serial.print(downPosition);
//  Serial.print(COMMA);
//  Serial.print(upPosition);
//  Serial.println(CMD_END);

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
  executing = true;
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
  
  executing = false;  
}

void exec_changeLength(float a, float b) {
  int speed = motorA.speed();
  motorA.moveTo(a);
  motorB.moveTo(b);
  if (usingAcceleration) {
    while (motorA.distanceToGo() != 0 || motorB.distanceToGo() != 0)
    {
      motorA.run();
      motorB.run();
    }
  }
  else {
    while (motorA.distanceToGo() != 0 || motorB.distanceToGo() != 0)
    {
#ifdef DEBUG
      Serial.print("Distancetogo: ");
      Serial.print(motorA.distanceToGo());
      Serial.print(",");
      Serial.println(motorB.distanceToGo());
#endif
      motorA.setSpeed(speed);
      motorB.setSpeed(speed);
      motorA.runSpeedToPosition();
      motorB.runSpeedToPosition();
    }
  }
  
  Serial.print("Deviation A, B: ");
  Serial.print(motorA.computeDeviation());
  Serial.print(", ");
  Serial.println(motorB.computeDeviation());
}

void exec_changeLengthCartesianMm(float x, float y) {
  float pA = getMachineA(x, y);
  float pB = getMachineB(machineWidth, x, y);
  pA = mmToMotorSteps(pA);
  pB = mmToMotorSteps(pB);
  exec_changeLength(pA, pB);
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
  float margin = 20.0; // 50mm margin is ridiculous

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
    if (abs(deltaX) > abs(deltaY))
    {
      // slope <=1 case    
      while ((abs(deltaX)/linesegs) > maxSegmentLength)
      {
        linesegs++;
      }
    }
    else
    {
      // slope >1 case
      while ((abs(deltaY)/linesegs) > maxSegmentLength)
      {
        linesegs++;
      }
    }
    
    // reduce delta to one line segments' worth.
    deltaX = deltaX/linesegs;
    deltaY = deltaY/linesegs;
  
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
      runSpeed = exec_desiredSpeed(linesegs, runSpeed, accel*4);
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

