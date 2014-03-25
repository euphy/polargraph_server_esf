/**
*  Polargraph Server for ATMEGA1280+ 
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  http://code.google.com/p/polargraph/

Specific features for Polarshield / arduino mega.
Implementation.

So this file is the interface for the extra features available in the 
mega/polarshield version of the polargraph.

*/

/*  Implementation of executeCommand for MEGA-sized boards that
have command storage features. */
void impl_processCommand(String inCmd, String inParam1, String inParam2, String inParam3, String inParam4, int inNoOfParams)
{
  impl_executeCommand(inCmd, inParam1, inParam2, inParam3, inParam4, inNoOfParams);
}

/**  
*  This includes the extra commands the MEGA is capable of handling.
*  It tries to run the command using the core executeBasicCommand
*  first, but if that doesn't work, then it will go through
*  it's own decision tree to try and run one of the additional
*  routines.
*/
void impl_executeCommand(String inCmd, String inParam1, String inParam2, String inParam3, String inParam4, int inNoOfParams)
{
  Serial.print(F("Looking to execute: "));
  Serial.print(inCmd);
  Serial.print(F(", p1:"));
  Serial.print(inParam1);
  Serial.print(F(", p2:"));
  Serial.print(inParam2);
  Serial.print(F(", p3:"));
  Serial.print(inParam3);
  Serial.print(F(", p4:"));
  Serial.println(inParam4);

  if (exec_executeBasicCommand(inCmd, inParam1, inParam2, inParam3, inParam4, inNoOfParams))
  {
    // that's nice, it worked
    Serial.println(F("Executed basic."));
  }
  else if (inCmd.startsWith(CMD_AUTO_CALIBRATE)) {
    Serial.println("calib.");
    motors_calibrateHome();
  }
  else
  {
    comms_unrecognisedCommand(inCmd, inParam1, inParam2, inParam3, inParam4, inNoOfParams);
    comms_ready();
  }
  inNoOfParams = 0;
}

/** Polarshield implementation of runBackgroundProcesses. This is basically stuff to do with 
the screen.
*/
void impl_runBackgroundProcesses()
{
//  motorA.correctDeviation();
//  motorB.correctDeviation();
  if (heartbeat.check()) {
    comms_ready();
  }
//  comms_checkForCommand();

  long motorCutoffTime = millis() - lastActivityTime;
  if ((automaticPowerDown) && powerOn && (motorCutoffTime > idleTimeBeforePowerDown))
  {
    Serial.println("Powering down because of inactivity.");
    motors_release();
  }
}

void impl_loadMachineSpecFromEeprom()
{
}


