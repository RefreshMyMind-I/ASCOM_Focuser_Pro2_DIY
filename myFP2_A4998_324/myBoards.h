//-----------------------------------------------------------------------
// myFocuserPro2 Driver Board Definitions
// (c) R, Brown, 2014-2022, All rights reserved.
//-----------------------------------------------------------------------
#ifndef myBoards_h
#define myBoards_h

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
#define MAXSTEPMODE         16
#endif

#define A4998DIR            3
#define A4998STEP           4
#define A4998ENABLE         8
#define A4998M2             5
#define A4998M1             6
#define A4998M0             7

#define MOTORPULSETIME      3

#define A4998FAST           6000
#define A4998MED            1200
#define A4998SLOW           18000


//-----------------------------------------------------------------------
// DRIVERBOARD CLASS DEFINITION
//-----------------------------------------------------------------------
class DriverBoard
{
  public:
    DriverBoard(byte);
    void init_hpsw(void);
    bool get_hpsw(void);

    // get
    int  getstepmode(void);         //
    byte getmotorspeed(void);       //
    int getstepdelay(void);

    // set
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
    byte boardtype;
    int  stepmode;
    int  stepdelay;                 // time in microseconds to wait between pulses when moving

    bool _motortimer_state;
};
#endif
