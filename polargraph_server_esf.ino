#include <Encoder.h>
#include <AccelStepperEncoder.h>

#define DEBUG

const String FIRMWARE_VERSION_NO = "2.0";

// for working out CRCs
static PROGMEM prog_uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

/*==========================================================================
    ELECTRICAL DETAILS and PHYSICAL SIZES
  ========================================================================*/

// encoders, left and right
Encoder encA(18, 19);
Encoder encB(6, 5);

// AccelStepperEncoder objects
AccelStepperEncoder motorA(AccelStepperEncoder::DRIVER, 14, 15, 1, 1, false);
AccelStepperEncoder motorB(AccelStepperEncoder::DRIVER, 1, 2, 1, 1, false);

// Endstop pins
int leftEndStopPin = 21;
int rightEndStopPin = 22;

// definition of the motor and drive train
float motorToEncoderRatio = 1.51;
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

// Equilibrium position - where the pen was when it was turned on.
float equilibriumA = 0;
float equilibriumB = 0;


/*==========================================================================
    CONTROL VARIABLES... on or off, how fast
  ========================================================================*/

// variable that controls whether the motors step or not.
volatile boolean runningMotors = true;

// interval timer that will run the stepper motors...
IntervalTimer motorTimer;

// ... the rate it'll do it at (microseconds)
int motorRunRate = 1000;
// ... and the function that get's called by it
void runMotors(void) {
  if (runningMotors) {
    motorB.run();
    motorA.run();
  }
}

// Motor speeds
long maxSpeed = 3000;
long accel = 3000;

// whether to use acceleration in accelstepper
boolean usingAcceleration = true;

// Timestamp which is set when some thing happens. 
// Used to determine whether to go to sleep or not.
volatile long lastActivityTime = 0L;

// period between status rebroadcasts
long comms_rebroadcastStatusInterval = 2000;

// Whether to kill the motors after inactivity
boolean automaticPowerDown = true;
// length of time since last activity before killing the motors.
long idleTimeBeforePowerDown = 600000L;


// Bunch of useful conversion functions. Nothing complicated here, but
// nice to have them wrapped up in a meaningful name.
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



/*==========================================================================
    POLARGRAPH COMMANDS, a subset.
  ========================================================================*/

const static String COMMA = ",";
const static String CMD_END = ",END";
const static String CMD_CHANGELENGTH = "C01";
const static String CMD_SETPOSITION = "C09";
const static String CMD_PENDOWN = "C13";
const static String CMD_PENUP = "C14";
const static String CMD_CHANGELENGTHDIRECT = "C17";
const static String CMD_SETMACHINESIZE = "C24";
const static String CMD_SETMACHINENAME = "C25";
const static String CMD_GETMACHINEDETAILS = "C26";
const static String CMD_RESETEEPROM = "C27";
const static String CMD_SETMACHINEMMPERREV = "C29";
const static String CMD_SETMACHINESTEPSPERREV = "C30";
const static String CMD_SETMOTORSPEED = "C31";
const static String CMD_SETMOTORACCEL = "C32";
const static String CMD_SETPENLIFTRANGE = "C45";
const static String CMD_SET_ROVE_AREA = "C21";
const static String CMD_RANDOM_DRAW = "C36";
const static String CMD_CHANGELENGTH_RELATIVE = "C40";

/*==========================================================================
    COMMUNICATION PROTOCOL, how to chat
  ========================================================================*/

// max length of incoming command
const int INLENGTH = 70;
const char INTERMINATOR = 10;

// reserve some characters
static String inCmd = "                                                  ";
static String inParam1 = "              ";
static String inParam2 = "              ";
static String inParam3 = "              ";
static String inParam4 = "              ";

// set to true if the last command was parsed safely and the next slot in 
// the buffer can be allocated.
boolean commandConfirmed = false;
boolean usingCrc = false;

// command buffer
const byte CMD_BUFFER_SIZE = 5;
static String* commandsBuffered[CMD_BUFFER_SIZE];
static byte bufferLength = 0;

static byte bufferReadIndex = 0; // pointer to current command
static byte bufferWriteIndex = 0; // index to next command to overwrite

// prevents new commands from being added to the queue
static boolean waitUntilAllCommandsAreParsed = false;




void setup() {
  Serial.begin(9600);
  delay(3000); 
  Serial.println("Polargraph Pro");
  pinMode(rightEndStopPin, INPUT); 
  pinMode(leftEndStopPin, INPUT); 

  // set up motorA
  motorA.setPinsInverted(false, false, true);
  motorA.setEnablePin(17);
  motorA.addEncoder(&encA, motorToEncoderRatio);
  motorA.setAcceleration(accel);
  motorA.setMaxSpeed(maxSpeed);
  
  // setup motorB
  motorB.setPinsInverted(true, false, true);
  motorB.setEnablePin(4);
  motorB.addEncoder(&encB, motorToEncoderRatio);
  motorB.setAcceleration(accel);
  motorB.setMaxSpeed(maxSpeed);

  motorTimer.begin(runMotors, motorRunRate);
  
  motors_calibrateHome();
}


/* Convert cartesian coords to native coords.
All of these kinematic functions are unit-agnostic, that is, 
if you put in mm, you'll get mm out, and if you put in eSteps,
you'll get eSteps out. Etc. 
*/
float getMachineA(float cX, float cY)
{
  float a = sqrt(sq(cX)+sq(cY));
  return a;
}
float getMachineB(float width, float cX, float cY)
{
  float b = sqrt(sq((width)-cX)+sq(cY));
  return b;
}

/* Convert native coords (cord lengths) into cartesian coords.
*/
float getCartesianX(float width, float aPos, float bPos)
{
  float calcX = (sq((float)width) - sq((float)bPos) + sq((float)aPos)) / ((float)width*2.0);
  return calcX;  
}
float getCartesianY(float cX, float aPos) 
{
  float calcY = sqrt(sq(aPos)-sq(cX));
  return calcY;
}

void loop() {
  comms_commandLoop();
}

