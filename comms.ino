/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  http://code.google.com/p/polargraph/

Comms.

This is one of the core files for the polargraph server program.  
Comms can mean "communications" or "commands", either will do, since
it contains methods for reading commands from the serial port.

*/

void comms_commandLoop() {
  while (true) {
    nextCommand = comms_readFromSerial();
    if (commandConfirmed) {
      Serial.println("Command Confirmed.");
      paramsExtracted = comms_parseCommand(nextCommand);
      if (paramsExtracted) {
        nextCommand = "";
        comms_ready();
        comms_executeParsedCommand();
      }
      else
      {
        Serial.print(F("Command not parsed."));
      }
    }
  }
}

char* comms_readFromSerial()
{
  // send ready
  // wait for instruction
  int idleTime = millis();
  
  // do this bit until we get a command confirmed
  // idle
  char* inS;

  // loop while there's no commands coming in
  Serial.println("Entering read from serial loop");
  while (strlen(inS) == 0)
  {
    impl_runBackgroundProcesses();
    // idle time is spent in this loop.
    int timeSince = millis() - idleTime;
    if (timeSince > comms_rebroadcastStatusInterval)
    {
      comms_ready();
      idleTime = millis();
    }
    
    // and now read the command if one exists
    // this also sets usingCrc AND commandConfirmed
    // to true or false
    inS = comms_readCommand();

    // if it's using the CRC check, then confirmation is easy
    if (inS != "" && !commandConfirmed) {
      comms_requestResend();
      inS = "";
    }
  }
  
  
  // CRC was ok, or we aren't using one
  idleTime = millis();
  lastActivityTime = idleTime;
  return inS;
}

boolean comms_parseCommand(char* inS)
{
  if (strstr(inS, CMD_END) != NULL)
  {
    comms_extractParams(inS);
    return true;
  }
  else 
    return false;
}  

char* comms_readCommand()
{
  // check if data has been sent from the computer:
  char inString[INLENGTH+1];
  int inCount = 0;
  while (Serial.available() > 0)
  {
    char ch = Serial.read();       // get it
    delay(1);
    inString[inCount] = ch;
    if (ch == INTERMINATOR)
    {
      Serial.flush();
      break;
    }
    inCount++;
  }
  inString[inCount] = 0;                     // null terminate the string
  String inS = inString;

  // check the CRC for this command
  // and set commandConfirmed true or false
  char* t1 = strtok(inString, ":");
  char* t2 = strtok(NULL, ":");
  commandConfirmed = true;  
  if (strlen(t1) > 0)
    Serial.println(t1);
  return t1;
}

void comms_executeParsedCommand()
{
  if (!executing && paramsExtracted)
  {
    executing = true;
    impl_processCommand(inCmd, inParam1, inParam2, inParam3, inParam4, inNoOfParams);
  }
  paramsExtracted = false;
  inNoOfParams = 0;
  
}


void comms_extractParams(char* inS) {
  
  // get number of parameters
  // by counting commas
  int length = strlen(inS);
  
  int startPos = 0;
  int paramNumber = 0;
  char* param = strtok(inS, ",");
  while (param != NULL) {
      switch(paramNumber) {
        case 0:
          inCmd = param;
          break;
        case 1:
          inParam1 = param;
          break;
        case 2:
          inParam2 = param;
          break;
        case 3:
          inParam3 = param;
          break;
        case 4:
          inParam4 = param;
          break;
        default:
          break;
      }
      paramNumber++;
      param = strtok(NULL, ",");
  }
  inNoOfParams = paramNumber;
  
    Serial.print(F("Command:"));
    Serial.print(inCmd);
    Serial.print(F(", p1:"));
    Serial.print(inParam1);
    Serial.print(F(", p2:"));
    Serial.print(inParam2);
    Serial.print(F(", p3:"));
    Serial.print(inParam3);
    Serial.print(F(", p4:"));
    Serial.println(inParam4);
}


long asLong(String inParam)
{
  char paramChar[inParam.length() + 1];
  inParam.toCharArray(paramChar, inParam.length() + 1);
  return atol(paramChar);
}
int asInt(String inParam)
{
  char paramChar[inParam.length() + 1];
  inParam.toCharArray(paramChar, inParam.length() + 1);
  return atoi(paramChar);
}
byte asByte(String inParam)
{
  int i = asInt(inParam);
  return (byte) i;
}
float asFloat(String inParam)
{
  char paramChar[inParam.length() + 1];
  inParam.toCharArray(paramChar, inParam.length() + 1);
  return atof(paramChar);
}

void comms_establishContact() 
{
  comms_ready();
}
void comms_ready()
{
//  comms_reportPosition();
  Serial.println(READY);
}
void comms_drawing()
{
  Serial.println(DRAWING);
}
void comms_requestResend()
{
  Serial.println(RESEND);
}
void comms_unrecognisedCommand(String inCmd, String inParam1, String inParam2, String inParam3, String inParam4, String inNoOfParams)
{
  Serial.print(F("Sorry, "));
  Serial.print(F("Command:"));
  Serial.print(inCmd);
  Serial.print(F(", p1:"));
  Serial.print(inParam1);
  Serial.print(F(", p2:"));
  Serial.print(inParam2);
  Serial.print(F(", p3:"));
  Serial.print(inParam3);
  Serial.print(F(", p4:"));
  Serial.println(inParam4);
  Serial.println(F(" isn't a command I recognise."));
}  

void comms_reportPosition()
{
  if (reportingPosition)
  {
    float cX = getCartesianX(machineWidth, encoderStepsToMm(motorA.readEnc()), encoderStepsToMm(motorB.readEnc()));
    float cY = getCartesianY(cX, encoderStepsToMm(motorA.readEnc()));
    Serial.print("CARTESIAN,");
    Serial.print(cX);
    Serial.print(COMMA);
    Serial.print(cY);
    Serial.println(CMD_END);
  
    //outputAvailableMemory();
  }
}


