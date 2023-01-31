#ifndef myi2ckeypad_h
#define myi2ckeypad_h

#include <inttypes.h>

// Author Robert Brown, 2016, All rights reserved
// Files for myFocuserPro2 I2C 4x4 Keypad matrix option
// Version 1.0

class i2ckeypad {
public:
  i2ckeypad(int);
  i2ckeypad(int, int, int);
  char get_key();
  void init();

private:
  void pcf8574_write(int, int);
  int pcf8574_byte_read(int);
};

#endif
