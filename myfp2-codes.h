myfp2-codes.h


Coil Power
CPW000			Coil Power off


Display
DIS001			initialise display
DIS002			update display


DriverBoard
BRD001			load driver board start
BRD002			load driver board done
BRD003			init home position switch
BRD004			read home position switch
BRD005			init motor timer
BRD006			start motor timer
BRD007			stop motor timer


Focuser Config
CFG001 			read focuser config
CFG002			focuser config not found
CFG003			save focuser config


InOutLEDs
LED001			initialise LEDs


Machine States
MAS001			apply backlash
MAS002			moving
MAS003			Find HomePosition: MoveIN till closed
MAS004			HomePosition MoveIN ERROR: HOMESTEPS exceeded
MAS005			HomePosition MoveIN stepstaken=
MAS006			HomePosition MoveIN finished
MAS007			>SetHomePosition
MAS008			HomePosition Move out till switch is OPEN
MAS009			HomePosition MoveOUT ERROR: HOMESTEPS exceeded
MAS010			HomePosition MoveOUT stepstaken=
MAS011			HomePosition MoveOUT ended
MAS012			>DelayAfterMove
MAS013			>FinishedMove
MAS014			>Idle
MAS015 			move complete
MAS016			HomePosition found, position >= 0
MAS017			HomePosition not found, position=0
MAS018			Find Home Position


Park
PAR000			Not parked
PAR001			Parked


Serial Comms
SER001			initialise serial comms
SER002			-Send:
SER003			recstr=
SER004			cmdstr=
SER005			WorkStr=


STEPPER-POWER
STP001			initialise STEPPER-POWER Detect


Temperature Probe
TMP001			initialise Temperature Probe
TMP002			Start temp sensor
TMP003			Sensors:
TMP004			Probe not found
TMP005			temp=
TMP006			read temperature

