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
  float x = asFloat(inParam1);
  float y = asFloat(inParam2);
  
  float a = getMachineA(x, y);
  float b = getMachineB(machineWidth, x, y);
  
  motorA.moveTo(mmToMotorSteps(a));
  motorB.moveTo(mmToMotorSteps(b));
}

/**
This moves the gondola in a straight line between p1 and p2.  Both input coordinates are in 
the native coordinates system.  

The fidelity of the line is controlled by maxLength - this is the longest size a line segment is 
allowed to be.  1 is finest, slowest.  Use higher values for faster, wobblier.
*/
void exec_drawStraightToPoint()
{
  // ok, we're going to plot some dots between p1 and p2.  Using maths. I know! Brave new world etc.
  
  // First, convert these values to cartesian coordinates
  // We're going to figure out how many segments the line
  // needs chopping into.
  float c2x = asFloat(inParam1);
  float c2y = asFloat(inParam2);
  c2x = mmToMotorSteps(c2x);
  c2y = mmToMotorSteps(c2y);
  
  float c1x = getCartesianX(machineWidth, encoderStepsToMm(motorA.readEnc()), encoderStepsToMm(motorB.readEnc()));
  float c1y = getCartesianY(c1x, encoderStepsToMm(motorA.readEnc()));
  c1x = mmToMotorSteps(c1x);
  c1y = mmToMotorSteps(c1y);
  
  float machineWidthSteps = mmToMotorSteps(machineWidth);
  float machineHeightSteps = mmToMotorSteps(machineHeight);
  float margin = mmToMotorSteps(50.0); // 50mm margin is ridiculous

#ifdef DEBUG
  Serial.print("From coords: ");
  Serial.print(c1x);
  Serial.print(",");
  Serial.println(c1y);
  Serial.print("To coords: ");
  Serial.print(c2x);
  Serial.print(",");
  Serial.println(c2y);
#endif

  // test to see if it's on the page
  // AND ALSO TO see if the current position is on the page.
  // Remember, the native system can easily specify points that can't exist,
  // particularly up at the top.
  if (c2x > margin 
    && c2x<machineWidthSteps-margin 
    && c2y > margin 
    && c2y <machineHeightSteps-margin
    && c1x > margin 
    && c1x<machineWidthSteps-margin 
    && c1y > margin 
    && c1y <machineHeightSteps-margin 
    )
    {
    reportingPosition = false;
    float deltaX = c2x-c1x;    // distance each must move (signed)
    float deltaY = c2y-c1y;
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
      Serial.println(linesegs);
#endif
      // compute next new location
      c1x = c1x + deltaX;
      c1y = c1y + deltaY;
  
      // convert back to machine space
      float pA = getMachineA(c1x, c1y);
      float pB = getMachineB(machineWidthSteps, c1x, c1y);
    
      // do the move
      runSpeed = exec_desiredSpeed(linesegs, runSpeed, accel*4);
      
#ifdef DEBUG
      Serial.print("Setting speed:");
      Serial.println(runSpeed);
#endif
      
      motorA.setSpeed(runSpeed);
      motorB.setSpeed(runSpeed);
      motorA.moveTo(pA);
      motorB.moveTo(pB);
  
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

