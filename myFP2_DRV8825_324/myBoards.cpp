//-----------------------------------------------------------------------
// myFocuserPro2 Driver Board Code
// (c) R, Brown, 2014-2022, All rights reserved.
//-----------------------------------------------------------------------
#include <Arduino.h>
#include "myDefines.h"
#include "myBoardDefs.h"
#include "myBoards.h"
#include "focuserconfig.h"
#ifdef HOMEPOSITIONSWITCH
#include <Bounce2.h>                      // needed to debounce Home Position switch
Bounce hpswbounce = Bounce();             // setup debouncer for hp switch
#endif
#include <TimerOne.h>                     // https://github.com/PaulStoffregen/TimerOne

// v1 of the home position switch which has switch normally open to D12 and ground
// when home position switch is NOT activated (contacts open), D12 = 5V = HIGH
// when home position switch is activated (contacts closed), D12 = GND = LOW
// Normal operation, home switch not activated, contacts open, D12 high, motor can move
// Fault operation, D12 (disconnected switch or power) floats high, motor can move

//-----------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern struct config_t myfocuser;
extern long fcurrentposition;                  // current focuser position
extern long ftargetposition;                   // target position
extern byte isMoving;                          // is the motor currently moving
extern byte MainStateMachine;
extern byte movedirection;                     // holds direction of new planned move
extern DriverBoard* driverboard;
extern void MotorStepClockwise(void);
extern void MotorStepAntiClockwise(void);


//-----------------------------------------------------------------------
// Local Data
//-----------------------------------------------------------------------
volatile bool hpswstate;

//-----------------------------------------------------------------------
// void timer_stepmotor(void)
// Run by Timer1, steps motor till fcurrentposition == ftargetposition
//-----------------------------------------------------------------------
void timer_stepmotor()
{
  if (fcurrentposition == ftargetposition)  // must come first else cannot halt
  {
    driverboard->stop_motortimer();
    DebugPrint("MAS013");
    MainStateMachine = State_DelayAfterMove;
    DebugPrint("MAS010");
  }
  else
  {
    // focuser not finished the move yet
      ( movedirection == MOVINGIN ) ? MotorStepAntiClockwise() : MotorStepClockwise();
    (movedirection == MOVINGIN) ? fcurrentposition-- : fcurrentposition++;
    // no need for validation as target has already been validated before move

#ifdef HOMEPOSITIONSWITCH
    // if moving in. check if hpsw closed or position = 0
    if (movedirection == MOVINGIN)
    {
      // if switch state = CLOSED and position >= 0
      // need to back OUT a little till switch opens and then set position to 0
      if ((hpswstate == HPSWCLOSED) && (fcurrentposition >= 0))
      {
        driverboard->stop_motortimer();
        isMoving = 1;
        fcurrentposition = ftargetposition = 0;
        MainStateMachine = State_SetHomePosition;
        DebugPrintln("MAS014");
        DebugPrintln("MAS006");
      }
      // else if switchstate = OPEN and Position = 0
      // need to move IN a little till switch CLOSES then
      else if ((hpswstate == HPSWOPEN) && (fcurrentposition == 0))
      {
        driverboard->stop_motortimer();
        isMoving = 1;
        fcurrentposition = ftargetposition = 0;
        MainStateMachine = State_FindHomePosition;
        DebugPrintln("MAS015");
        DebugPrintln("MAS016");
      }
    }   // if ( movedirection == MOVINGIN )
#endif  // home position switch
  }
}

//-----------------------------------------------------------------------
// DRIVERBOARD CLASS
//-----------------------------------------------------------------------
DriverBoard::DriverBoard(byte brdtype) : boardtype(brdtype)
{
  if ( boardtype == DRV8825 || boardtype == DRV8825RE)
  {
    pinMode(DRV8825ENABLE, OUTPUT);
    pinMode(DRV8825DIR, OUTPUT);
    pinMode(DRV8825STEP, OUTPUT);
    digitalWrite(DRV8825ENABLE, HIGH);
    pinMode(DRV8825M2, OUTPUT);
    pinMode(DRV8825M1, OUTPUT);
    pinMode(DRV8825M0, OUTPUT);
  }
  else if ( boardtype == DRV8825TFT22)
  {
    pinMode(DRV8825ENABLETFT, OUTPUT);
    pinMode(DRV8825DIRTFT, OUTPUT);
    pinMode(DRV8825STEPTFT, OUTPUT);
    digitalWrite(DRV8825ENABLETFT, HIGH);
    pinMode(DRV8825M2TFT, OUTPUT);
    pinMode(DRV8825M1TFT, OUTPUT);
    pinMode(DRV8825M0TFT, OUTPUT);
  }
  hpswstate = false;
  DebugPrintln("BRD003");
  init_hpsw();
  DebugPrintln("BRD004");
  hpswstate = get_hpsw();
  setstepmode(myfocuser.stepmode);          // sets stepmode
  setmotorspeed(myfocuser.motorspeed);      // sets delay
  _motortimer_state = false;
}

void DriverBoard::init_hpsw(void)
{
#ifdef HOMEPOSITIONSWITCH
  pinMode( HPSWPIN, INPUT_PULLUP);          // set up the Home Position Switch pin as an input
  hpswbounce.attach( HPSWPIN );             // setup defaults for debouncing hp Switch
  hpswbounce.interval( 5 );                 // sets debounce time
#endif
}

bool DriverBoard::get_hpsw(void)
{
#ifdef HOMEPOSITIONSWITCH                   // hpsw open state is HIGH, closed state = LOW
  hpswbounce.update();                      // we need to call update to read home position switch state, no interrupts are used
  hpswstate = hpswbounce.read();            // reads the home position switch state
#endif
  return hpswstate;
}

//-----------------------------------------------------------------------
// void driverboard->set_motor_power(bool);
// turn coil power on, turn coil power off
//-----------------------------------------------------------------------
void DriverBoard::set_motor_power(bool state)
{
  if ( state == true )
  {
    // power on
    if ( boardtype == DRV8825 || boardtype == DRV8825RE)
    {
      digitalWrite(DRV8825ENABLE, LOW);
    }
    else if ( boardtype == DRV8825TFT22)
    {
      digitalWrite(DRV8825ENABLETFT, LOW);
    }
    delay(1);                             // need to wait 1ms before driver chip is ready for stepping
  }
  else
  {
    // power off
    if ( this->boardtype == DRV8825 || this->boardtype == DRV8825RE)
    {
      digitalWrite(DRV8825ENABLE, HIGH);
    }
    else if ( boardtype == DRV8825TFT22)
    {
      digitalWrite(DRV8825ENABLETFT, HIGH);
    }
  }
}

//DONE
void DriverBoard::set_motortimer()
{
  long stepdelay;
  // timer on, this is a move
  switch (driverboard->getstepmode())
  {
    case STEP1:
      stepdelay = (long)driverboard->getstepdelay();
      break;
    case STEP2:
      stepdelay = (long)driverboard->getstepdelay() / 2;
      break;
    case STEP4:
      stepdelay = (long)driverboard->getstepdelay() / 4;
      break;
    case STEP8:
      stepdelay = (long)driverboard->getstepdelay() / 8;
      break;
    case STEP16:
      stepdelay = (long)driverboard->getstepdelay() / 8;
      break;
    case STEP32:
      stepdelay = (long)driverboard->getstepdelay() / 8;
      break;
    default:
      stepdelay = (long)driverboard->getstepdelay();
      break;
  }
  DebugPrintln("BRD006");
  Timer1.setPeriod(stepdelay);
  Timer1.stop();
  _motortimer_state = false;  
}

//-----------------------------------------------------------------------
// void driverboard->init_motortimer(void);
// initialise motor timer
//-----------------------------------------------------------------------
void DriverBoard::init_motortimer(void)
{
  Timer1.initialize();
  set_motortimer();
  Timer1.attachInterrupt(timer_stepmotor);
  Timer1.stop();
  _motortimer_state = false;
  DebugPrintln("BRD005");
}

//-----------------------------------------------------------------------
// bool driverboard->get_motortimer_state(void);
// return motor timer state
//-----------------------------------------------------------------------
bool DriverBoard::get_motortimer_state(void)
{
  return _motortimer_state;
}

//-----------------------------------------------------------------------
// void driverboard->start_motortimer(void);
// enable motor timer, stepper starts moving
//-----------------------------------------------------------------------
void DriverBoard::start_motortimer(void)
{
  long stepdelay;
  // timer on, this is a move
  switch (driverboard->getstepmode())
  {
    case STEP1:
      stepdelay = (long)driverboard->getstepdelay();
      break;
    case STEP2:
      stepdelay = (long)driverboard->getstepdelay() / 2;
      break;
    case STEP4:
      stepdelay = (long)driverboard->getstepdelay() / 4;
      break;
    case STEP8:
      stepdelay = (long)driverboard->getstepdelay() / 8;
      break;
    case STEP16:
      stepdelay = (long)driverboard->getstepdelay() / 8;
      break;
    case STEP32:
      stepdelay = (long)driverboard->getstepdelay() / 8;
      break;
    default:
      stepdelay = (long)driverboard->getstepdelay();
      break;
  }
  DebugPrintln("BRD006");
  Timer1.setPeriod(stepdelay);
  Timer1.start();
  _motortimer_state = true;
}

//-----------------------------------------------------------------------
// void driverboard->stop_motortimer(void);
// stop motor timer, stepper stops moving
//-----------------------------------------------------------------------
void DriverBoard::stop_motortimer(void)
{
  // timer off, end of move
  Timer1.stop();
  _motortimer_state = false;
  DebugPrintln("BRD007");
}

//-----------------------------------------------------------------------
// void driverboard->movemotor(byte);
// move stepper motor in the specified direction
//-----------------------------------------------------------------------
void DriverBoard::movemotor(byte ddir)
{
  if ( this->boardtype == DRV8825 || this->boardtype == DRV8825RE )
  {
    digitalWrite(DRV8825DIR, ddir);
    digitalWrite(DRV8825STEP, HIGH);
    delayMicroseconds(MOTORPULSETIME);
    digitalWrite(DRV8825STEP, LOW);
  }
  else if ( boardtype == DRV8825TFT22)
  {
    digitalWrite(DRV8825DIRTFT, ddir);
    digitalWrite(DRV8825STEPTFT, HIGH);
    delayMicroseconds(MOTORPULSETIME);
    digitalWrite(DRV8825STEPTFT, LOW);
  }
#ifdef HOMEPOSITIONSWITCH
  if ( ddir == MOVINGIN )
  {
    hpswstate = get_hpsw();
  }
#endif
}

int DriverBoard::getstepmode(void)
{
  return this->stepmode;
}

void DriverBoard::setstepmode(int smode)
{
  switch (this->boardtype)
  {
    case DRV8825:
    case DRV8825RE:
      switch (smode)
      {
        case 1:
          digitalWrite(DRV8825M0, 0);
          digitalWrite(DRV8825M1, 0);
          digitalWrite(DRV8825M2, 0);
          break;
        case 2:
          digitalWrite(DRV8825M0, 1);
          digitalWrite(DRV8825M1, 0);
          digitalWrite(DRV8825M2, 0);
          break;
        case 4:
          digitalWrite(DRV8825M0, 0);
          digitalWrite(DRV8825M1, 1);
          digitalWrite(DRV8825M2, 0);
          break;
        case 8:
          digitalWrite(DRV8825M0, 1);
          digitalWrite(DRV8825M1, 1);
          digitalWrite(DRV8825M2, 0);
          break;
        case 16:
          digitalWrite(DRV8825M0, 0);
          digitalWrite(DRV8825M1, 0);
          digitalWrite(DRV8825M2, 1);
          break;
        case 32:
          digitalWrite(DRV8825M0, 1);
          digitalWrite(DRV8825M1, 0);
          digitalWrite(DRV8825M2, 1);
          break;
        default:    // full stepping
          digitalWrite(DRV8825M0, 0);
          digitalWrite(DRV8825M1, 0);
          digitalWrite(DRV8825M2, 0);
          break;
      }
      break;
    case DRV8825TFT22:
      switch (smode)
      {
        case 1:
          digitalWrite(DRV8825M0TFT, 0);
          digitalWrite(DRV8825M1TFT, 0);
          digitalWrite(DRV8825M2TFT, 0);
          break;
        case 2:
          digitalWrite(DRV8825M0TFT, 1);
          digitalWrite(DRV8825M1TFT, 0);
          digitalWrite(DRV8825M2TFT, 0);
          break;
        case 4:
          digitalWrite(DRV8825M0TFT, 0);
          digitalWrite(DRV8825M1TFT, 1);
          digitalWrite(DRV8825M2TFT, 0);
          break;
        case 8:
          digitalWrite(DRV8825M0TFT, 1);
          digitalWrite(DRV8825M1TFT, 1);
          digitalWrite(DRV8825M2TFT, 0);
          break;
        case 16:
          digitalWrite(DRV8825M0TFT, 0);
          digitalWrite(DRV8825M1TFT, 0);
          digitalWrite(DRV8825M2TFT, 1);
          break;
        case 32:
          digitalWrite(DRV8825M0TFT, 1);
          digitalWrite(DRV8825M1TFT, 0);
          digitalWrite(DRV8825M2TFT, 1);
          break;
        default:    // full stepping
          digitalWrite(DRV8825M0TFT, 0);
          digitalWrite(DRV8825M1TFT, 0);
          digitalWrite(DRV8825M2TFT, 0);
          break;
      }
      break;
  }
  this->stepmode      = smode;
  myfocuser.stepmode  = smode;
}

int DriverBoard::getstepdelay(void)
{
  return this->stepdelay;
}

void DriverBoard::setstepdelay(int sdelay)
{
  this->stepdelay = sdelay;
}

byte DriverBoard::getmotorspeed(void)
{
  byte retspd = FAST;

  switch ( this->stepdelay)
  {
    case DRVFAST :
      retspd = FAST;
      break;
    case DRVMED :
      retspd = MED;
      break;
    case DRVSLOW :
      retspd = SLOW;
      break;
    default:
      // do nothing
      break;
  }
  return retspd;
}

void DriverBoard::setmotorspeed(byte spd )
{
  switch ( spd )
  {
    case SLOW:
      this->stepdelay = DRVSLOW;
      break;
    case MED:
      this->stepdelay = DRVMED;
      break;
    case FAST:
      this->stepdelay = DRVFAST;
      break;
    default:
      // do nothing
      break;
  }
}
