readme.txt
----------

ClockClock Master project with the NXP LPC845-BRK board.

Links:
- https://github.com/ErichStyger/MetaClockClock


Notes:
USART0 is on 25/24 to the host usb cdc.

Commands sent from the master:
rs sendcmd 16 stepper m 0 0 359 10 cw
rs sendcmd 16 stepper m 0 0 0 10 cc

rs sendcmd 16 stepper m 0 1 359 10 cw
rs sendcmd 16 stepper m 0 1 0 10 cc

rs sendcmd 16 rs status

rs sendcmd 16 stepper help
stepper                   ; Group of stepper commands
  help|status             ; Print help or status information
  reset                   ; Performs a X12.017 driver reset
  zero all                ; Move all motors to zero position using magnet sensor
  zero <c> <m>            ; Move clock (0-3) and motor (0-1) to zero position using magnet sensor
  offs <c> <m> <v>        ; Set offset value for clock (0-3) and motor (0-1)
  offs 12                 ; Calculate offset from 12-o-clock
  delay <m0>...<m7>       ; Set delay for motors (0..7)
  step <c> <m> <steps>    ; Perform a number of (blocking) steps for clock (0-3) and motor (0-1)
  goto <c> <m> <pos>      ; Goto position (non-blocking) for clock (0-3) and motor (0-1)
  angle <c> <m> <deg> <md>; Move clock (0-3) and motor (0-1) to angle (0..359) using mode (cc, cw, sh)
  a <md> <degs>           ; Move clock hands to angles using mode (cc, cw, sh)
  m <c> <m> <a> <d> <md>  ; Move the clock and motor to the angle with delay using mode (cc, cw, sh), uppercase mode letter is without accel control
  q <c> <m> <cmd>         ; Queue a command for clock and motor
  exq                     ; Execute command in queue
  idle                    ; Check if steppers are idle


@19 1 cmd stepper q 0 1 stepper m 0 1 0 30 sh
@0 1 cmd stepper exq


Format on the bus:
@<dst> <src> cmd   <cr>
