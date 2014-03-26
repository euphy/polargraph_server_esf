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

void comms_checkForCommand() {
  if (!commandConfirmed) {
    if (Serial.available() > 0)
    {
#ifdef DEBUG_COMMS_BUFF
      Serial.print("Buf: ");
      Serial.println(Serial.available());
      Serial.print("Pos:");
      Serial.println(bufferPosition);
#endif
      char ch = Serial.read();       // get it
      nextCommand[bufferPosition] = ch;
#ifdef DEBUG_COMMS_BUFF
      Serial.print("Command so far: ");
      Serial.println(nextCommand);
#endif
      if (ch == INTERMINATOR)
      {
        nextCommand[bufferPosition] = 0;                     // null terminate the string
        if (strlen(nextCommand) <= 0) {
          Serial.print("Resend (got): ");
          Serial.println(nextCommand);
          comms_requestResend();
          nextCommand[0] = NULL;
          commandConfirmed = false;
        }
        else {
#ifdef DEBUG_COMMS
          Serial.print("Confirmed: ");
          Serial.println(nextCommand);
#endif
          commandConfirmed = true;
          bufferPosition = 0;
          comms_ready();          
        }
      }
      else if (ch >= 40) {
        bufferPosition++;
        if (bufferPosition > INLENGTH)
        { // if the command is too big, chuck it out!
          nextCommand[0] = NULL;
          commandConfirmed = false;
          bufferPosition = 0;
        }
      }
      lastActivityTime = millis();
    }
  }
}

void comms_clearParams() {
  strcpy(inCmd, "");
  strcpy(inParam1, "");
  strcpy(inParam2, "");
  strcpy(inParam3, "");
  strcpy(inParam4, "");
  inNoOfParams = 0;
}
  
void comms_commandLoop() {
  while (true) {
    impl_runBackgroundProcesses();
    if (commandConfirmed) {
#ifdef DEBUG_COMMS
      Serial.print(F("Command Confirmed: "));
      Serial.println(nextCommand);
#endif
      paramsExtracted = comms_parseCommand(nextCommand);
      if (paramsExtracted) {
        Serial.println(F("Params extracted."));
        strcpy(nextCommand, "");
        commandConfirmed = false;
        comms_ready(); // signal ready for next
        comms_executeParsedCommand();
        comms_clearParams();
      }
      else
      {
        Serial.println(F("Command not parsed."));
        strcpy(nextCommand, "");
        comms_clearParams();
        commandConfirmed = false;
      }
    }
  }
}

boolean comms_parseCommand(char * inS)
{
#ifdef DEBUG_COMMS
  Serial.print("parsing inS: ");
  Serial.println(inS);
#endif
  char * comp = strstr(inS, CMD_END);
  if (comp != NULL)
  {
#ifdef DEBUG_COMMS
    Serial.println(F("About to extract params"));
#endif
    comms_extractParams(inS);
    return true;
  }
  else if (comp == NULL) {
    Serial.println(F("IT IS NULL!"));
    return false;
  }
  else {
    Serial.println(F("Could not parse command."));
    return false;
  }
}

void comms_extractParams(char* inS) {
  
  // get number of parameters
  // by counting commas
  int length = strlen(inS);
  
  int startPos = 0;
  int paramNumber = 0;
  char * param = strtok(inS, COMMA);
#ifdef DEBUG_COMMS
  Serial.print(F("Param "));
  Serial.print(paramNumber);
  Serial.print(": ");
  Serial.println(param);
#endif
  while (param != 0) {
      switch(paramNumber) {
        case 0:
          strcpy(inCmd, param);
          break;
        case 1:
          strcpy(inParam1, param);
          break;
        case 2:
          strcpy(inParam2, param);
          break;
        case 3:
          strcpy(inParam3, param);
          break;
        case 4:
          strcpy(inParam4, param);
          break;
        default:
          break;
      }
      paramNumber++;
      param = strtok(NULL, COMMA);
#ifdef DEBUG_COMMS
      Serial.print(F("Param "));
      Serial.print(paramNumber);
      Serial.print(": ");
      Serial.print(param);
      Serial.print("... ");
      for (int i = 0; i<sizeof(param); i++) {
        Serial.print(":");
        Serial.print((int)param[i]);
      }
      Serial.println(".");
#endif
  }
  inNoOfParams = paramNumber;
#ifdef DEBUG_COMMS
    Serial.print(F("Command:"));
    Serial.print(inCmd);
    Serial.print(F(", p1:"));
    Serial.print(inParam1);
    Serial.print(F(", p2:"));
    Serial.print(inParam2);
    Serial.print(F(", p3:"));
    Serial.print(inParam3);
    Serial.print(F(", p4:"));
    Serial.print(inParam4);
    Serial.print(F(", inNoOfParams "));
    Serial.println(inNoOfParams);
#endif
}

void comms_executeParsedCommand()
{
#ifdef DEBUG_COMMS
  Serial.print(F("Executing: "));
  Serial.println(executing);
  Serial.print(F("Params extracted: "));
  Serial.println(paramsExtracted);
#endif
  if (!executing && paramsExtracted)
  {
    impl_processCommand(inCmd, inParam1, inParam2, inParam3, inParam4, inNoOfParams);
    paramsExtracted = false;
    inNoOfParams = 0;
  }
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
  comms_reportPosition();
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
  Serial.print(F("Sorry, command: "));
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
    if (isCalibrated) {
      float cX = getCartesianX(machineWidth, encoderStepsToMm(motorA.readEnc()), encoderStepsToMm(motorB.readEnc()));
      float cY = getCartesianY(cX, encoderStepsToMm(motorA.readEnc()));
      Serial.print("CARTESIAN,");
      Serial.print(cX);
      Serial.print(COMMA);
      Serial.print(cY);
      Serial.println(CMD_END);
    }
    else {
      Serial.println(F("Not calibrated."));
    }
  }
}


