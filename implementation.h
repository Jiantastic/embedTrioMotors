#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include "mbed.h"
#include "QEI.h"
#include "PID.h"

//**********************************************
//*            Constants Definitions           *
//**********************************************
//Photointerrupter input pins
#define I1pin D2
#define I2pin D11
#define I3pin D12

//Incremental encoder input pins
#define CHA D7
#define CHB D8

//Motor Drive output pins   //Mask in output byte
#define L1Lpin D4  //0x01
#define L1Hpin D5  //0x02
#define L2Lpin D3  //0x04
#define L2Hpin D6  //0x08
#define L3Lpin D9  //0x10
#define L3Hpin D10 //0x20


// Extern definitions
extern RawSerial pc;

extern int8_t intState;
extern int8_t intStateOld;
extern int8_t orState; //Rotor offset at motor state 0

extern float lastPosition;
extern float currentPosition;
extern float currentRPSValue;                // global angular velocity/RPS value

//float rotations=5; //Set a random starting variable - replaced by regex later on
extern float targetPosition;  //30*117=3510 100*117=11700
extern float PIDrate;
extern float Kc;
extern float Ti;
extern float Td;
extern float speedControl;
extern float dutyCycle; //global duty cycle to be passed to motorOut
extern bool AUTO;
extern PID controller;
extern Thread PIDthread;

//@@@@@@@ Photointerrupter inputs @@@@@@@
// DigitalIn I1(I1pin);
//Define I1 as interrupt inputs
extern DigitalIn I1;
extern DigitalIn I2;
extern DigitalIn I3;

extern Ticker sampleRPS;
extern Ticker PrintRPS;
//Timer speedTimer;

//@@@@@@@ QEI inputs @@@@@@@ NOT USED
extern DigitalIn CHAInput;
extern DigitalIn CHBInput;


// //Motor Drive outputs
extern DigitalOut *L1Ldigi;
extern DigitalOut *L1Hdigi;
extern DigitalOut *L2Ldigi;
extern DigitalOut *L2Hdigi;
extern DigitalOut *L3Ldigi;
extern DigitalOut *L3Hdigi;

extern PwmOut *L1Lpwm;
extern PwmOut *L1Hpwm;
extern PwmOut *L2Lpwm;
extern PwmOut *L2Hpwm;
extern PwmOut *L3Lpwm;
extern PwmOut *L3Hpwm;

// QEI config
extern QEI wheel;



// *** CONSTANT VALUES ***

//Drive state to output table
const int8_t driveTable[] = {0x12, 0x18, 0x09, 0x21, 0x24, 0x06, 0x00, 0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07, 0x05, 0x03, 0x04, 0x01, 0x00, 0x02, 0x07};
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//@@@@@@@ Phase lead to make motor spin @@@@@@@ Must change according to PID output value
const int8_t lead = -1; //2 for forwards, -2 for backwards

// New Constants(not constants with regex)

extern float Vref;      // speed to be changed by regex
extern float Rref;      // rotation to be changed by regex
const float thresholdRPS = 0.1;
// rate at which interrupt is called, also the time difference(t) since the last interrupt call
const float RPS_SAMPLING_RATE = 0.1;  


//------- Convert photointerrupter inputs to a rotor state -------
inline int8_t readRotorState()
{
    return stateMap[I1 + 2 * I2 + 4 * I3];
}

// List of functions
float modulus(float n);
void motorOut(int8_t driveState);
int8_t motorHome();
void getRPSfromQEI();
void controlInit();
void controlR();
void printRPSfromQEI();
void readPIrunMotor();

#endif
