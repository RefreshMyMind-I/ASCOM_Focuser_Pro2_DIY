//-----------------------------------------------------------------------
// myFocuserPro2 Driver Board Definitions
// (c) R, Brown, 2014-2022, All rights reserved.
//-----------------------------------------------------------------------
#ifndef myBoards_h
#define myBoards_h

#include <Arduino.h>

#ifndef SLOW
#define SLOW                0 // motorspeeds
#endif
#ifndef MED
#define MED                 1
#endif
#ifndef FAST
#define FAST                2
#endif

#ifndef STEP1
#define STEP1               1             // stepmodes
#endif
#ifndef STEP2
#define STEP2               2
#endif
#ifndef STEP4
#define STEP4               4
#endif
#ifndef STEP8
#define STEP8               8
#endif
#ifndef STEP16
#define STEP16              16
#endif
#ifndef STEP32
#define STEP32              32
#endif
#ifndef STEP64
#define STEP64              64
#endif
#ifndef STEP128
#define STEP128             128
#endif
#ifndef STEP256
#define STEP256             256        
#endif
#ifndef MAXSTEPMODE
#define MAXSTEPMODE         32
#endif

#define DRV8825DIR          3             // drv8825 control lines
#define DRV8825STEP         4
#define DRV8825ENABLE       8
#define DRV8825M2           7             // for easydriver, do not connect
#define DRV8825M1           6             // for easydriver, connect to MS1
#define DRV8825M0           5             // for easydriver, connect to MS0

// TFT drv pins
#define DRV8825DIRTFT       A4
#define DRV8825STEPTFT      3
#define DRV8825ENABLETFT    A5
#define DRV8825M2TFT        10            // for easydriver, do not connect
#define DRV8825M1TFT        9             // for easydriver, connect to MS1
#define DRV8825M0TFT        8             // for easydriver, connect to MS0
 
#define MOTORPULSETIME      3             // drv8825 driver board

#define DRVFAST             5000          // delays for motorspeed
#define DRVMED              10000
#define DRVSLOW             15000

class DriverBoard
{
  public:
    DriverBoard(byte);
    void init_hpsw(void);
    bool get_hpsw(void);
    
    // getter
    int  getstepmode(void);         //
    byte getmotorspeed(void);       //
    int  getstepdelay(void);
    
    // setter
    void setstepmode(int);
    void setstepdelay(int);         // this controls the speed of the motor, using delayMicroseconds(int) after each step
    void setmotorspeed(byte);       // set motorspeed

    void set_motor_power(bool);

    void set_motortimer(void);
    void init_motortimer(void);
    bool get_motortimer_state(void);
    void start_motortimer(void);
    void stop_motortimer(void);
    void movemotor(byte);           // move motor one step, takes parameter direction 0 | 1

  private:
    byte boardtype;                 // DRV8825
    int  stepmode;                  //
    int  stepdelay;                 // time in microseconds to wait between pulses when moving

    bool _motortimer_state;
};
#endif

/* these are on the header of the ILI9341 tft
  2.2-SPI 320x240 TFT MODULE
  VCC=5V
  GND=GND
  HEADER FTF                    Arduino
  Pin1  VCC       3.3-5V        5V
  Pin2  GND                     GND
  Pin3  CS        3.3-5V        5
  Pin4  RESET     3.3-5V        4
  Pin5  DC        3.3-5V        6
  Pin6  SDI/MOSI  3.3-5V        11 MOSI
  Pin7  SCK       3.3V-5V       13 SCLK
  Pin8  LED/BL    3.3-5V        D7
  Pin9  SDO/MISO  -   -         FOR SD CARD
*/

/* Arduino Nano Pin Mappings - Unable to use I2C
  A0  Push Buttons IN/OUT
  A1  Blue  LED
  A2  Green LED
  A3  Buzzer
  A4  DRV8825 DIR
  A5  DRV8825 ENABLE
  A6  -
  A7  -
  D2  DS18B20 Temperature Probe
  D3  DRV8825 STEP
  D4  2.2TFT RESET
  D5  2.2TFT CS
  D6  2.2TFT DC
  D7  2.2TFT LED
  D8  DRV8825 MS0
  D9  DRV8825 MS1
  D10 DRV8825 MS2
  D11 2.2TFT  SDI/MOSI
  D12 -
  D13 2.2TFT  SCK
*/
