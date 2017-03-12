#ifndef IMPLEMENTATION_H
#define	IMPLEMENTATION_H

#include "mbed.h"
#include "QEI.h"
#include "PID.h"

//Photointerrupter input pins
#define I1pin D2
#define I2pin D11
#define I3pin D12

//Incremental encoder input pins
#define CHA   D7
#define CHB   D8  

//Motor Drive output pins   //Mask in output byte
#define L1Lpin D4           //0x01
#define L1Hpin D5           //0x02
#define L2Lpin D3           //0x04
#define L2Hpin D6           //0x08
#define L3Lpin D9           //0x10
#define L3Hpin D10          //0x20

//Mapping from sequential drive states to motor phase outputs
/*
State   L1  L2  L3
0       H   -   L
1       -   H   L
2       L   H   -
3       L   -   H
4       -   L   H
5       H   L   -
6       -   -   -
7       -   -   -
*/
//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};  
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//Phase lead to make motor spin
const int8_t lead = -2;  //2 for forwards, -2 for backwards

//Status LED
extern DigitalOut led1;

//Photointerrupter inputs
extern DigitalIn I1;
extern DigitalIn I2;
extern DigitalIn I3;

// UNCOMMENT : Incremental encoder inputs
// DigitalIn CHAInput(CHA);
// DigitalIn CHBInput(CHB);

//Motor Drive outputs
extern DigitalOut L1L;
extern DigitalOut L1H;
extern DigitalOut L2L;
extern DigitalOut L2H;
extern DigitalOut L3L;
extern DigitalOut L3H;

//Position Encoder outputs
extern DigitalOut CHAInput;
extern DigitalOut CHBInput;

void inputHandler();
void rotationHandler();
void controlAlgorithm();
void musicHandler();
void motorOut(int8_t driveState);
void readPositionEncoderState();
int8_t motorHome();
void readPhotoInterrupterState();
void getRPMFromPositionEncoder();

// serial config
extern Serial pc;

// QEI position encoder config
extern QEI wheel;

// PID controller
extern PID myPID;
extern float Kc;
extern float Ti;
extern float Td;
extern float Setpoint;
extern float Input;
extern float Output;
extern PwmOut pwm1;
extern float testOut;
extern PwmOut pwm2;


//Convert photointerrupter inputs to a rotor state, 1 2 and 4 and binary powers
inline int8_t readRotorState(){
    return stateMap[I1 + 2*I2 + 4*I3];
}

extern int8_t orState;
extern int8_t intState;
extern int8_t intStateOld;

// global states for calculating RPM with position encoder
extern float lastPosition;
extern float currentPosition;
const extern float RPM_SAMPLING_RATE;
extern float currentRPMValue;

#endif
