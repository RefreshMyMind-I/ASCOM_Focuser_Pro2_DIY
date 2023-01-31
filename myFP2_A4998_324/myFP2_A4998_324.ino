// -----------------------------------------------------------------------------
// myFOCUSERPRO2 OFFICIAL FIRMWARE RELEASE 324
// (c) Copyright Robert Brown 2014-2022. All Rights Reserved.
// (c) Copyright Holger, 2019-2021. All Rights Reserved.
// (c) Joël Collet, move-timer, 2021-2022, All rights reserved.
// -----------------------------------------------------------------------------
// Supports driver boards A4998

// CONTRIBUTIONS
// Please support the ongoing development of this project by using PayPal
// (paypal.com) to send your donation to user rbb1brown@gmail.com (Robert Brown).
// All contributions are gratefully accepted and go directly to supporting this
// and other projects.

// -----------------------------------------------------------------------------
// PCB BOARDS
// -----------------------------------------------------------------------------
// A4998-M-MT-F-BT https://sourceforge.net/projects/arduinoascomfocuserpro2diy/files/Gerber%20Files/A4998/

// Stripboard/Layout
// myFocuser_hw203_A4988_MT_schem  https://sourceforge.net/projects/arduinoascomfocuserpro2diy/files/DRIVER%20BOARDS/BOARD%20A4998/


// -----------------------------------------------------------------------------
// SPECIFY DRIVER BOARD IN focuserconfig.h
// -----------------------------------------------------------------------------
// DRIVER BOARDS - Please specify your driver board in focuserconfig.h
// only 1 can be defined


// -----------------------------------------------------------------------------
// SPECIFY FOCUSER CONFIG
// -----------------------------------------------------------------------------
// Go to file focuserconfig.h and enable the hardware options you have


// -----------------------------------------------------------------------------
// FIRMWARE CODE START - INCLUDES AND LIBRARIES
// -----------------------------------------------------------------------------
// Project specific includes

#include <Arduino.h>
#include <myQueue.h>                      //  By Steven de Salas
#include <myEEPROM.h>                     // needed for EEPROM
#include <myeepromanything.h>             // needed for EEPROM
#include <TimerOne.h>
#include "myDefines.h"
#include "focuserconfig.h"
#include "myBoards.h"


// -----------------------------------------------------------------------------
// DRIVER BOARD DEFINITIONS - DO NOT CHANGE
// -----------------------------------------------------------------------------
char programName[]  = "myFP2A4998";
DriverBoard* driverboard;

//-----------------------------------------------------------------------
// EXTERN DATA - DO NOT CHANGE
//-----------------------------------------------------------------------
extern void  timer_stepmotor(void);
extern volatile bool hpswstate;
extern byte  tprobe1;
extern float lasttemp;

//-----------------------------------------------------------------------
// GLOBAL DATA - DO NOT CHANGE
//-----------------------------------------------------------------------
struct config_t myfocuser;
char programVersion[] = "324";
char ProgramAuthor[]  = "(c) R BROWN 2020";
byte MainStateMachine;

long fcurrentposition;                  // current focuser position
long ftargetposition;                   // target position
byte isMoving;                          // is the motor currently moving
byte tempcompavailable;
int  currentaddr;                       // will be address in eeprom of the data stored
byte writenow;                          // should we update values in eeprom
byte jogging;
byte joggingDirection;                  // defined jogging direction, 0 = IN, 1 = OUT
byte movedirection;                     // holds direction of new planned move
byte stepperpower;                      // stepperpower state 0 or 1
Queue<String> queue(QUEUELENGTH);       // receive serial queue of commands
String line;
String btline;
byte delayeddisplayupdatestatus;

unsigned long lasteepromwrite;

#ifdef BLUETOOTH
#include <SoftwareSerial.h>               // needed for bt adapter - this library is already included when you install the Arduino IDE

SoftwareSerial btSerial( BTTX, BTRX);
#endif

// -----------------------------------------------------------------------------
// CODE START
// -----------------------------------------------------------------------------

#include "display.h"
#include "temperature.h"
#include "serialcomms.h"

// STEPPER MOTOR ROUTINES
void MotorStepClockwise()
{
#ifdef INOUTLEDS
  (!myfocuser.reversedirection) ? digitalWrite( INLED, 1) : digitalWrite( OUTLED, 1);
#endif
  (!myfocuser.reversedirection) ? driverboard->movemotor(1) : driverboard->movemotor(0);
#ifdef INOUTLEDS
  (!myfocuser.reversedirection) ? digitalWrite( INLED, 0 ) : digitalWrite( OUTLED, 0);
#endif
}

void MotorStepAntiClockwise()
{
#ifdef INOUTLEDS
  (!myfocuser.reversedirection) ? digitalWrite( OUTLED, 1) : digitalWrite( INLED, 1);
#endif
  (!myfocuser.reversedirection) ? driverboard->movemotor(0) : driverboard->movemotor(1);
#ifdef INOUTLEDS
  (!myfocuser.reversedirection) ? digitalWrite( OUTLED, 0 ) : digitalWrite( INLED, 0);
#endif
}

void writeEEPROMNow()
{
  EEPROM_writeAnything(currentaddr, myfocuser);       // update values in EEPROM
  writenow = 0;
}

void UpdateEEPROMCheck(void)
{
  // only called if writenow is 1
  unsigned long currentMillis = millis();
  if (((currentMillis - lasteepromwrite) > EEPROMWRITEINTERVAL) || (currentMillis < lasteepromwrite))
  {
    lasteepromwrite = currentMillis;          // update the timestamp
    myfocuser.validdata = VALIDDATAFLAG;
    myfocuser.fposition = fcurrentposition;
    writeEEPROMNow();                         // update values in EEPROM
    DebugPrint("CFG003");
  }
}

void setfocuserdefaults()
{
  myfocuser.validdata = VALIDDATAFLAG;
  myfocuser.maxstep   = DEFALTMAXSTEP;
  myfocuser.fposition = DEFAULTPOSITION;
  // set to 1 or ON
  myfocuser.displayenabled = myfocuser.tcdirection = myfocuser.tempmode = myfocuser.stepmode = myfocuser.coilpower = 1;
  // set to 0 or OFF
  isMoving = myfocuser.backlash_out_enabled = myfocuser.backlash_in_enabled = myfocuser.backlashsteps_out = myfocuser.backlashsteps_in = myfocuser.delayaftermove = myfocuser.tempcoefficient = myfocuser.tempcompenabled = myfocuser.stepsizeenabled = myfocuser.reversedirection = 0;
  myfocuser.lcdpagetime       = DISPLAYPAGETIMEMIN;
  myfocuser.stepsize          = DEFAULTSTEPSIZE;      // default is celsius
  myfocuser.tempresolution    = TEMPRESOLUTION;
  myfocuser.focuserdirection  = MOVINGIN;
  myfocuser.motorspeed        = FAST;
  myfocuser.pbsteps           = 1;
  resetdisplayoption();
  writeEEPROMNow();                                   // update values in EEPROM
}

// reboot the Arduino
void software_Reboot()
{
  asm volatile ( "jmp 0");                            // jump to the start of the program
}

// read the push button switches and return state of switches
// 1 = SW1 ON AND SW2 OFF, 2 = SW2 ON AND SW1 OFF, 3 = SW1 ON and SW2 ON, 0 = OFF
int readpbswitches()
{
  // for 1.2K sw1 (681) 650-720, sw2 (338) 310-380, sw1 and sw2 (509) 460-530
  // for 1K sw1 (630) 600-660, sw2 (335) 300-370, sw1 and sw2 (510) 480-540
#ifdef PUSHBUTTONS
  int val = analogRead(PBSWITCHESPIN);    // read the input pin
  if ( val >= 650 && val <= 720 )
  {
    return 1;                             // toggle sw1 ON and SW2 OFF
  }
  else if ( val >= 460 && val <= 530 )
  {
    return 3;                             // toggle sw1 and sw2 ON
  }
  else if ( val >= 310 && val <= 380 )
  {
    return 2;                             // toggle sw2 ON and SW1 OFF
  }
#endif // #ifdef PUSHBUTTONS
  return 0;
}

void updatepushbuttons(void )
{
#ifdef PUSHBUTTONS
  int PBVal = readpbswitches();
  delay(20);                                          // wait small delay in case user is holding pb down
  if ( readpbswitches() == PBVal )                    // same key pressed
  {
    switch ( PBVal )                                  // now check the pbval using a switch for 1 2 and 3
    {
      case 0:
        return;
        break;
      case 1:                                         // toggle sw1 is ON and 2 is off
        ftargetposition = ftargetposition - myfocuser.pbsteps;
        ftargetposition = (ftargetposition < 0) ? 0 : ftargetposition;
        writenow = 1;
        break;
      case 2:                                         // toggle sw2 is ON and SW1 is OFF
        ftargetposition = ftargetposition + myfocuser.pbsteps;
        ftargetposition = (ftargetposition > myfocuser.maxstep) ? myfocuser.maxstep : ftargetposition;
        writenow = 1;
        break;
      case 3:                                         // toggle sw1 and sw2 are ON
#ifdef BUZZER
        digitalWrite(BUZZERPIN, 1);                   // turn on buzzer
#endif
        while ( readpbswitches() == 3 )               // wait for pb to be released
          ;
        fcurrentposition = 0;
        ftargetposition = 0;
#ifdef BUZZER
        digitalWrite(BUZZERPIN, 0);                   // turn off buzzer
#endif
        writenow = 1;
        break;
      default:
        // do nothing
        break;
    } // end of switch
#ifdef LCDDISPLAY
    if ( myfocuser.displayenabled == 1)
    {
      updatepositionlcd();
    }
#endif
#ifdef OLEDDISPLAY
    if ( myfocuser.displayenabled == 1)
    {
      updatepositionoled();
    }
#endif
  }
#endif // #ifdef PUSHBUTTONS
}

void updatejogging(void)
{
  if ( joggingDirection == 0 )
  {
    ftargetposition = ftargetposition - 1;
    ftargetposition = (ftargetposition < 0) ? 0 : ftargetposition;
#ifdef LCDDISPLAY
    updatepositionlcd();
#endif
#ifdef OLEDDISPLAY
    updatepositionoled();
#endif
    writenow = 1;
  }
  else
  {
    ftargetposition = ftargetposition + 1;
    ftargetposition = (ftargetposition > myfocuser.maxstep) ? myfocuser.maxstep : ftargetposition;
#ifdef LCDDISPLAY
    updatepositionlcd();
#endif
#ifdef OLEDDISPLAY
    updatepositionoled();
#endif
    writenow = 1;
  }
#ifdef SUPERSLOWJOGGING
  delay(SLOWSPEEDJOGDELAY);
#endif // SUPERSLOWJOGGING
}

void updatestepperpowerdetect()
{
#ifdef STEPPERPWRDETECT
  stepperpower = (analogRead( STEPPERDETECTPIN )) > 600 ? 1 : 0;
  // for ULN2003 powered from  9V with 4.7VZ, reading was 3.72V = 763
  // for drv8825 powered from 12V with 4.7VZ, reading was 4.07V = 834
  // Each digit = .00488millivolts
#else
  stepperpower = 1;
#endif
}

// Setup
void setup()
{
  int datasize;                             // will hold size of the struct myfocuser - 6 bytes
  int nlocations;                           // number of storage locations available in EEPROM
  byte found;

  line = "";
#ifdef BLUETOOTH
  btline = "";
#endif

  Serial.begin(SERIALPORTSPEED);            // initialize serial port
  clearSerialPort();                        // clear any garbage from serial buffer

#ifdef BLUETOOTH
  btSerial.begin(BTPORTSPEED);              // start bt adapter
  clearbtPort();
#endif

#ifdef BUZZER
  pinMode(BUZZERPIN, OUTPUT);               // turn ON the Buzzer - provide power ON beep
  digitalWrite( BUZZERPIN, 1);
#endif

#ifdef INOUTLEDS
  DebugPrintln("LED001");
  pinMode( INLED, OUTPUT);                  // turn ON both LEDS as power on cycle indicator
  pinMode( OUTLED, OUTPUT);
  digitalWrite( INLED, 1 );
  digitalWrite( OUTLED, 1 );
#endif

  stepperpower = 1;
#ifdef STEPPERPWRDETECT
  pinMode( STEPPERDETECTPIN, INPUT );
  DebugPrintln("STP001");
  updatestepperpowerdetect();
#endif

  isMoving = writenow = jogging = joggingDirection = tprobe1 = 0;
#ifdef DELAYEDDISPLAYUPDATE
  delayeddisplayupdatestatus = 1;
#else
  delayeddisplayupdatestatus = 0;
#endif

#ifdef LCDDISPLAY
  initlcd();
  DebugPrintln("DIS001");
#endif

#ifdef OLEDDISPLAY
  initoled();
  DebugPrintln("DIS001");
  DebugPrintln("DIS001");
#endif

  currentaddr = 0;                          // start at 0 if not found later
  found = 0;
  datasize = sizeof( myfocuser );
  nlocations = EEPROMSIZE / datasize;
  for (int lp1 = 0; lp1 < nlocations; lp1++ )
  {
    int addr = lp1 * datasize;
    EEPROM_readAnything( addr, myfocuser );
    if ( myfocuser.validdata == VALIDDATAFLAG ) // check to see if the data is valid
    {
      currentaddr = addr;                       // data was erased so write some default values
      found = 1;
      break;
    }
  }
  if ( found == 1 )
  {
    // set the focuser back to the previous settings
    // done after this in one hit
    // mark current eeprom address as invalid and use next one
    // each time focuser starts it will read current storage, set it to invalid, goto next location and
    // write values to there and set it to valid - so it doesnt always try to use same locations over and
    // over and destroy the eeprom
    // using it like an array of [0-nlocations], ie 100 storage locations for 1k EEPROM
    EEPROM_readAnything( currentaddr, myfocuser );
    myfocuser.validdata = 0;
    writeEEPROMNow();                       // update values in EEPROM
    currentaddr += datasize;                // goto next free address and write data
    // bound check the eeprom storage and if greater than last index [0-EEPROMSIZE-1] then set to 0
    if ( currentaddr >= (nlocations * datasize) )
    {
      currentaddr = 0;
    }
    myfocuser.validdata = VALIDDATAFLAG;
    writeEEPROMNow();                       // update values in EEPROM
  }
  else
  {
    // set defaults because not found
    DebugPrintln("CFG002");
    setfocuserdefaults();
  }

  DebugPrintln("TMP001");
  init_temp();

  tempcompavailable = 0;
  if ( tprobe1 == 1 )
  {
    tempcompavailable = 1;
  }

  myfocuser.tempcompenabled = 0;        // disable temperature compensation on startup else focuser will auto adjust whilst focusing!

  // range check focuser variables
  myfocuser.coilpower = myfocuser.coilpower & 0x01;
  myfocuser.reversedirection = myfocuser.reversedirection & 0x01;
  myfocuser.lcdpagetime = (myfocuser.lcdpagetime < DISPLAYPAGETIMEMIN) ? DISPLAYPAGETIMEMIN :  myfocuser.lcdpagetime;
  myfocuser.lcdpagetime = (myfocuser.lcdpagetime > DISPLAYPAGETIMEMAX) ? DISPLAYPAGETIMEMAX :  myfocuser.lcdpagetime;
  myfocuser.maxstep     = (myfocuser.maxstep < FOCUSERLOWERLIMIT ) ? FOCUSERLOWERLIMIT : myfocuser.maxstep;
  myfocuser.fposition   = (myfocuser.fposition < 0 ) ? 0 : myfocuser.fposition;
  myfocuser.fposition   = (myfocuser.fposition > myfocuser.maxstep ) ? myfocuser.maxstep : myfocuser.fposition;
  myfocuser.stepsize    = (myfocuser.stepsize <= 0 ) ? DEFAULTSTEPSIZE : myfocuser.stepsize;
  myfocuser.stepsize    = (myfocuser.stepsize > MAXIMUMSTEPSIZE ) ? MAXIMUMSTEPSIZE : myfocuser.stepsize;
  myfocuser.displayenabled = (myfocuser.displayenabled & 0x01);
  myfocuser.focuserdirection = myfocuser.focuserdirection & 0x01;
  if ( myfocuser.displayoption == 0)
  {
    resetdisplayoption();
  }
  movedirection = myfocuser.focuserdirection;

  DebugPrintln("BRD001");
  driverboard = new DriverBoard(DRVBRD);
  DebugPrintln("BRD002");

  if ( myfocuser.coilpower == 0)
  {
    driverboard->set_motor_power(false);
  }

  fcurrentposition = ftargetposition  = myfocuser.fposition;
  DebugPrint("fcurrentposition=");
  DebugPrint(fcurrentposition);
  DebugPrint(", ftargetposition=");
  DebugPrintln(ftargetposition);
  writenow = 1;

  // set up timer 
  driverboard->init_motortimer();
  
  if ( myfocuser.displayenabled == 0 )
  {
#ifdef LCDDISPLAY
    lcd->noDisplay();
    lcd->noBacklight();
#endif
#ifdef OLEDDISPLAY
    myoled->Display_Off();
#endif
  }

  DebugPrintln("TMP006");
  read_temp();

  lasteepromwrite = millis();

#ifdef INOUTLEDS
  digitalWrite( INLED, 0 );                // turn off the IN/OUT LEDS and BUZZER
  digitalWrite( OUTLED, 0 );
#endif

#ifdef BUZZER
  digitalWrite( BUZZERPIN, 0);
#endif

  MainStateMachine = State_Idle;
}

// Main Loop
void loop()
{
  static uint32_t timestampdelayaftermove = millis();       // for implementing delay after move wait time
  static uint32_t timestampdelayedupdate = millis();        // for updating display after a move has completed
  static uint32_t timestampendmove = millis();              // use for release of coil power
  static byte backlash_count = 0;
  static byte backlash_enabled = 0;
  static byte updatecount = 0;
  static bool parked = true;
#ifdef HOMEPOSITIONSWITCH
  static int stepstaken = 0;                // used with physical home position switch
#endif

#ifdef BLUETOOTH
  btSerialEvent();                          // check for command from bt adapter
#endif

  DebugPrintln("STP001");
  updatestepperpowerdetect();

  if ( queue.count() >= 1 )                 // check for serial command
  {
    ser_comms();
  }

  switch (MainStateMachine)
  {
    case State_Idle:
      if (fcurrentposition != ftargetposition)
      {
        isMoving = 1;                       // due to timing issue with TFT must be set here
        driverboard->set_motor_power(true);
        driverboard->set_motortimer();
        MainStateMachine = State_InitMove;
      }
      else
      {
        uint32_t timenow = millis();
        isMoving = 0;
        if ( delayeddisplayupdatestatus == 1)
        {
          // check if 5 seconds has elasped after move has finished
          if (((timenow - timestampdelayedupdate) > DISPLAYUPDATEWAIT) || (timenow < timestampdelayedupdate))
          {
            timestampdelayedupdate = timenow;

            update_temp();

            if ( myfocuser.displayenabled == 1)
            {
#ifdef LCDDISPLAY                   // update displays
              UpdateLCD();
#endif
#ifdef OLEDDISPLAY
              UpdateOLED();
#endif
#ifdef NOKIADISPLAY
              UpdateNOKIA();
#endif
#ifdef UTFTDISPLAY
              UpdateuTFT();
#endif
#ifdef TFTDISPLAY
              UpdateTFT();
#endif
            }
          } // if (((timenow - timestampdelayedupdate) > DISPLAYUPDATEWAIT) || (timenow < timestampdelayedupdate))
        } // if ( delayeddisplayupdatestatus == 1)
        else
        {
          if (tprobe1 == 1)
          {
            update_temp();
          }
          if ( myfocuser.displayenabled == 1)
          {
#ifdef LCDDISPLAY
            UpdateLCD();
#endif
#ifdef OLEDDISPLAY
            UpdateOLED();
#endif
#ifdef NOKIADISPLAY
            UpdateNOKIA();
#endif
#ifdef UTFTDISPLAY
            UpdateuTFT();
#endif
#ifdef TFTDISPLAY
            UpdateTFT();
#endif
          } // if ( myfocuser.displayenabled == 1)
        } // // if ( delayeddisplayupdatestatus == 1)

        updatepushbuttons();

        if ( jogging == 1)
        {
          updatejogging();
        } // if ( jogging == 1)

        if (parked == false)
        {
          uint32_t timenow = millis();
          if (((timenow - timestampendmove) > ENDMOVEDELAY) || (timenow < timestampendmove))
          {
            timestampendmove = timenow;         // update the timestamp
            // need to obey rule - can only release motor if coil power is disabled
            if ( myfocuser.coilpower == 0 )
            {
              driverboard->set_motor_power(false);
              DebugPrintln("CPW000");
            }
            DebugPrintln("PAR001");
            parked = true;
          } // if (parked == false)
        }

        if ( writenow == 1 )
        {
          UpdateEEPROMCheck();
        } // if ( writenow == 1 )
      } // if (fcurrentposition != ftargetposition)
      break;

    case State_InitMove:
      isMoving = 1;
      if ( ftargetposition < fcurrentposition )
      {
        movedirection = MOVINGIN;
        backlash_count = myfocuser.backlashsteps_in;
        backlash_enabled = myfocuser.backlash_in_enabled;
      }
      else
      {
        movedirection = MOVINGOUT;
        backlash_count = myfocuser.backlashsteps_out;
        backlash_enabled = myfocuser.backlash_out_enabled;
      }  
      // enable leds
#ifdef INOUTLEDS
      if (movedirection == MOVINGIN)
      {
        (!myfocuser.reversedirection) ? digitalWrite( OUTLED, 1) : digitalWrite( INLED, 1);
      }
      else  // moving out
      {
        (!myfocuser.reversedirection) ? digitalWrite( INLED, 1) : digitalWrite( OUTLED, 1);
      }
#endif
      if (movedirection != myfocuser.focuserdirection)
      {
        if ( backlash_enabled == 1 )
        {
          // apply backlash
          myfocuser.focuserdirection = movedirection;
          MainStateMachine = State_ApplyBacklash;
          DebugPrintln("MAS001");
        }
        else
        {
          // do not apply backlash, go straight to moving
          MainStateMachine = State_Moving;
          DebugPrintln("MAS002");
        }
      }
      else
      {
        MainStateMachine = State_Moving;
        DebugPrintln("MAS002");
      }
      break;

    case State_ApplyBacklash:
      if ( backlash_count )
      {
        (movedirection == MOVINGIN) ? MotorStepAntiClockwise() : MotorStepClockwise();
        delayMicroseconds((long)driverboard->getstepdelay());
        backlash_count--;;
      }
      else
      {
        MainStateMachine = State_Moving;
        DebugPrintln("MAS002");
      }
      break;

    case State_Moving:
      if ( driverboard->get_motortimer_state() == false)
      {
        // enable motor timer, start moving
        driverboard->set_motor_power(true);
        driverboard->start_motortimer();
      }

      // update display and temperature during a move??
      if ( myfocuser.lcdupdateonmove == 1 )
      {
        updatecount++;
        if ( updatecount > LCDUPDATESTEPCOUNT )
        {
          updatecount = 0;
          if ( delayeddisplayupdatestatus == 1)
          {
#ifdef LCDDISPLAY
            updatepositionlcd();
#endif
#ifdef OLEDDISPLAY
            updatepositionoled();
#endif
#ifdef NOKIADISPLAY
            updatepositionnokia();
#endif
#ifdef UTFTDISPLAY
            updatepositionutft();
#endif
#ifdef TFTDISPLAY
            updatepositiontft();
#endif
          } // if ( delayeddisplayupdatestatus == 1)
        } // if ( updatecount > LCDUPDATESTEPCOUNT )
      } // if ( myfocuser.lcdupdateonmove == 1 )
      break;

    case State_FindHomePosition:                // move in till home position switch closes
#ifdef HOMEPOSITIONSWITCH
      stepstaken = 0;
      DebugPrintln("MAS003");
      while ( hpswstate == HPSWOPEN )
      {
        // step IN till switch closes
        MotorStepAntiClockwise();
        delayMicroseconds(driverboard->getstepdelay());
        stepstaken++;
        if ( stepstaken > HOMESTEPS )           // this prevents the endless loop if the hpsw is not connected or is faulty
        {
          DebugPrintln("MAS004");
          break;
        }
      }
      DebugPrint("MAS005 ");
      DebugPrintln(stepstaken);
      DebugPrintln("MAS006");
#endif  // HOMEPOSITIONSWITCH
      MainStateMachine = State_SetHomePosition;
      DebugPrintln("MAS007");
      break;

    case State_SetHomePosition:                 // move out till home position switch opens
#ifdef HOMEPOSITIONSWITCH
      stepstaken = 0;
      DebugPrintln("MAS008");
      // if the previous moveIN failed at HOMESTEPS and HPSWITCH is still open then the
      // following while() code will drop through and have no effect and position = 0
      myfocuser.focuserdirection = !movedirection;
      while ( driverboard->get_hpsw() == HPSWCLOSED )
      {
        // step out till switch opens
        MotorStepClockwise();
        delayMicroseconds(driverboard->getstepdelay());
        stepstaken++;
        if ( stepstaken > HOMESTEPS )           // this prevents the endless loop if the hpsw is not connected or is faulty
        {
          DebugPrintln("MAS009");
          break;
        }
      }
      DebugPrint("MAS010 ");
      DebugPrintln(stepstaken);
      DebugPrintln("MAS011");
#endif // HOMEPOSITIONSWITCH
      timestampdelayaftermove = millis();
      MainStateMachine = State_DelayAfterMove;
      DebugPrintln("MAS012");
      break;

    case State_DelayAfterMove:
      {
        uint32_t timenow = millis();
        if ( ((timenow - timestampdelayaftermove) > myfocuser.delayaftermove) || (timenow < timestampdelayaftermove))
        {
          timestampdelayaftermove = timenow;
          MainStateMachine = State_FinishedMove;
          DebugPrintln("MAS013");
        }
      }
      break;

    case State_FinishedMove:
      // turn off leds
#ifdef INOUTLEDS
      digitalWrite( INLED,  0);
      digitalWrite( OUTLED, 0);
#endif
      if ( myfocuser.coilpower == 0 )
      {
        DebugPrintln("PAR000");
        parked = false;
        timestampendmove = millis();
      }
      if ( delayeddisplayupdatestatus == 1)
      {
        timestampdelayedupdate = millis();
      }
      isMoving = 0;
      writenow = 1;
      MainStateMachine = State_Idle;
      DebugPrintln("MAS014");
      break;

    default:
      MainStateMachine = State_Idle;
      break;
  }
} // end Loop()
