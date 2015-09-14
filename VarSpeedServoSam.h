/*
  Servo.h - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
  Copyright (c) 2009 Michael Margolis.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* 
  A servo is activated by creating an instance of the Servo class passing 
  the desired pin to the attach() method.
  The servos are pulsed in the background using the value most recently 
  written using the write() method.

  Note that analogWrite of PWM on pins associated with the timer are 
  disabled when the first servo is attached.
  Timers are seized as needed in groups of 12 servos - 24 servos use two 
  timers, 48 servos will use four.
  The sequence used to sieze timers is defined in timers.h

  The methods are:

    Servo - Class for manipulating servo motors connected to Arduino pins.

    attach(pin )  - Attaches a servo motor to an i/o pin.
    attach(pin, min, max  ) - Attaches to a pin setting min and max values in microseconds
    default min is 544, max is 2400  
 
    write()     - Sets the servo angle in degrees.  (invalid angle that is valid as pulse in microseconds is treated as microseconds)
    writeMicroseconds() - Sets the servo pulse width in microseconds 
    read()      - Gets the last written servo pulse width as an angle between 0 and 180. 
    readMicroseconds()   - Gets the last written servo pulse width in microseconds. (was read_us() in first release)
    attached()  - Returns true if there is a servo attached. 
    detach()    - Stops an attached servos from pulsing its i/o pin. 
 */

#ifndef VarSpeedServoSam_h
#define VarSpeedServoSam_h

#include <inttypes.h>

/* 
 * Defines for 16 bit timers used with  Servo library 
 *
 * If _useTimerX is defined then TimerX is a 16 bit timer on the current board
 * timer16_Sequence_t enumerates the sequence that the timers should be allocated
 * _Nbr_16timers indicates how many 16 bit timers are available.
 */

// Architecture specific include
#if defined(ARDUINO_ARCH_SAM)
 // copied from "sam/ServoTimers.h"
// For SAM3X:
#define _useTimer1
#define _useTimer2
#define _useTimer3
#define _useTimer4
#define _useTimer5

#if defined (_useTimer1)
#define TC_FOR_TIMER1       TC1
#define CHANNEL_FOR_TIMER1  0
#define ID_TC_FOR_TIMER1    ID_TC3
#define IRQn_FOR_TIMER1     TC3_IRQn
#define HANDLER_FOR_TIMER1  TC3_Handler
#endif
#if defined (_useTimer2)
#define TC_FOR_TIMER2       TC1
#define CHANNEL_FOR_TIMER2  1
#define ID_TC_FOR_TIMER2    ID_TC4
#define IRQn_FOR_TIMER2     TC4_IRQn
#define HANDLER_FOR_TIMER2  TC4_Handler
#endif
#if defined (_useTimer3)
#define TC_FOR_TIMER3       TC1
#define CHANNEL_FOR_TIMER3  2
#define ID_TC_FOR_TIMER3    ID_TC5
#define IRQn_FOR_TIMER3     TC5_IRQn
#define HANDLER_FOR_TIMER3  TC5_Handler
#endif
#if defined (_useTimer4)
#define TC_FOR_TIMER4       TC0
#define CHANNEL_FOR_TIMER4  2
#define ID_TC_FOR_TIMER4    ID_TC2
#define IRQn_FOR_TIMER4     TC2_IRQn
#define HANDLER_FOR_TIMER4  TC2_Handler
#endif
#if defined (_useTimer5)
#define TC_FOR_TIMER5       TC0
#define CHANNEL_FOR_TIMER5  0
#define ID_TC_FOR_TIMER5    ID_TC0
#define IRQn_FOR_TIMER5     TC0_IRQn
#define HANDLER_FOR_TIMER5  TC0_Handler
#endif

typedef enum { _timer1, _timer2, _timer3, _timer4, _timer5, _Nbr_16timers } timer16_Sequence_t ;

#else
#error "This library only supports boards with an SAM processor."
#endif

#define VarSpeedServoSam_VERSION           2     // software version of this library

#define MIN_PULSE_WIDTH       544     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2400     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define REFRESH_INTERVAL    20000     // minumim time to refresh servos in microseconds 

#define SERVOS_PER_TIMER       12     // the maximum number of servos controlled by one timer 
#define MAX_SERVOS   (_Nbr_16timers  * SERVOS_PER_TIMER)

#define INVALID_SERVO         255     // flag indicating an invalid servo index

#define CURRENT_SEQUENCE_STOP   255    // used to indicate the current sequence is not used and sequence should stop

typedef struct  {
  uint8_t nbr        :6 ;             // a pin number from 0 to 63
  uint8_t isActive   :1 ;             // true if this channel is enabled, pin not pulsed if false 
} ServoPin_t   ;  

typedef struct {
  ServoPin_t Pin;
  volatile unsigned int ticks;
  unsigned int target;      // Extension for slowmove
  uint8_t speed;          // Extension for slowmove
} servo_t;

typedef struct {
  uint8_t position;
  uint8_t speed;
} servoSequencePoint;

class VarSpeedServoSam
{
public:
  VarSpeedServoSam();
  uint8_t attach(int pin);           // attach the given pin to the next free channel, sets pinMode, returns channel number or 0 if failure
  uint8_t attach(int pin, int min, int max); // as above but also sets min and max values for writes. 
  void detach();
  void write(int value);             // if value is < 200 its treated as an angle, otherwise as pulse width in microseconds 
  void write(int value, uint8_t speed); // Move to given position at reduced speed.
          // speed=0 is identical to write, speed=1 slowest and speed=255 fastest.
          // On the RC-Servos tested, speeds differences above 127 can't be noticed,
          // because of the mechanical limits of the servo.
  void write(int value, uint8_t speed, bool wait); // wait parameter causes call to block until move completes
  void writeMicroseconds(int value); // Write pulse width in microseconds 
  void slowmove(int value, uint8_t speed);
  void stop(); // stop the servo where it is

  int read();                        // returns current pulse width as an angle between 0 and 180 degrees
  int readMicroseconds();            // returns current pulse width in microseconds for this servo (was read_us() in first release)
  bool attached();                   // return true if this servo is attached, otherwise false 

  uint8_t sequencePlay(servoSequencePoint sequenceIn[], uint8_t numPositions, bool loop, uint8_t startPos);
  uint8_t sequencePlay(servoSequencePoint sequenceIn[], uint8_t numPositions); // play a looping sequence starting at position 0
  void sequenceStop(); // stop movement
private:
   uint8_t servoIndex;               // index into the channel data for this servo
   int8_t min;                       // minimum is this value times 4 added to MIN_PULSE_WIDTH
   int8_t max;                       // maximum is this value times 4 added to MAX_PULSE_WIDTH
   servoSequencePoint * curSequence; // for sequences
   uint8_t curSeqPosition; // for sequences
};

#endif