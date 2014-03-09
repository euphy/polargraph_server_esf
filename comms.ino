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
#ifdef DEBUG_COMMS
      Serial.print("Serial available: ");
      Serial.println(Serial.available());
      Serial.print("Reading count ");
      Serial.println(bufferPosition);
#endif
      char ch = Serial.read();       // get it
      nextCommand[bufferPosition] = ch;
#ifdef DEBUG_COMMS
      Serial.print("Command in int: ");
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
          commandConfirmed = true;
          bufferPosition = 0;
        }
      }
      else {
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


void comms_commandLoop() {
  while (true) {
    delay(100);
    impl_runBackgroundProcesses();
    if (commandConfirmed) {
      Serial.print(F("Command Confirmed: "));
      Serial.println(nextCommand);
      paramsExtracted = comms_parseCommand(nextCommand);
      if (paramsExtracted) {
        Serial.println(F("Params extracted."));
        strcpy(nextCommand, "");
        commandConfirmed = false;
        comms_ready(); // signal ready for next
        comms_executeParsedCommand();
      }
      else
      {
        Serial.println(F("Command not parsed."));
      }
    }
  }
}

boolean comms_parseCommand(char * inS)
{
  Serial.print("parsing inS: ");
  Serial.println(inS);
  char * comp = strstr(inS, CMD_END);
  Serial.println(comp);
  Serial.println(CMD_END);
  
  if (comp != NULL)
  {
    Serial.println(F("About to extract params"));
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
  Serial.print(F("Param "));
  Serial.print(paramNumber);
  Serial.print(": ");
  Serial.println(param);
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
      Serial.print(F("Param "));
      Serial.print(paramNumber);
      Serial.print(": ");
      Serial.println(param);
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

void comms_executeParsedCommand()
{
  Serial.print(F("Executing: "));
  Serial.println(executing);
  Serial.print(F("Params extracted: "));
  Serial.println(paramsExtracted);
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
  }
}


