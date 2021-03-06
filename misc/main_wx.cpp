#include "mbed.h"
#include "rtos.h"
#include "PID.h"
#include "QEI.h"
// #include "implementation.h"
// #include "slre.h"
// #include <cstring>

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

//Status LED
DigitalOut led1(LED1);

//@@@@@@@ NEW CONSTANTS Definition @@@@@@@
RawSerial pc(SERIAL_TX, SERIAL_RX);

const float Vref = 3;
const float Rref = 30;

//const float thresholdRPS = 33;
//const float periodDC = 0.01; //Set duty cycle period to 0.01s (10ms)

int8_t intState = 0;
int8_t intStateOld = 0;
int8_t orState = 0; //Rotor offset at motor state 0

const float RPS_SAMPLING_RATE = 0.1;      // rate at which interrupt is called, also the time difference(t) since the last interrupt call
float lastPosition = 0;
float currentPosition = 0;
float currentRPSValue = 0;                // global angular velocity/RPS value

float rotations=5; //Set a random starting variable - replaced by regex later on
float targetPosition=117*Rref;
float PIDrate = 0.1;
float Kc = 10.0;
float Ti = 3.0;
float Td = 0;
float speedControl = 0;
float dutyCycle = 0; //global duty cycle to be passed to motorOut
bool AUTO = 1;
PID controller(Kc, Ti, Td, PIDrate);
Thread PIDthread;

//@@@@@@@ Photointerrupter inputs @@@@@@@
// DigitalIn I1(I1pin);
//Define I1 as interrupt inputs
DigitalIn I1(I1pin);
DigitalIn I2(I2pin);
DigitalIn I3(I3pin);

Ticker sampleRPS;
//Timer speedTimer;

//@@@@@@@ QEI inputs @@@@@@@ NOT USED
DigitalIn CHAInput(CHA);
DigitalIn CHBInput(CHB);

// //Motor Drive outputs
// DigitalOut *L1Ldigi = new DigitalOut(L1Lpin);
// DigitalOut L1Hdigi(L1Hpin);
// //DigitalOut *L1Hdigi = new DigitalOut(L1Hpin);
// DigitalOut *L2Ldigi = new DigitalOut(L2Lpin);
// DigitalOut L2Hdigi(L2Hpin);
// //DigitalOut *L2Hdigi = new DigitalOut(L2Hpin);
// DigitalOut *L3Ldigi = new DigitalOut(L3Lpin);
// DigitalOut L3Hdigi(L3Hpin);
// //DigitalOut *L3Hdigi = new DigitalOut(L3Hpin);

PwmOut *L1Lpwm = new PwmOut(L1Lpin);
PwmOut *L1Hpwm = new PwmOut(L1Hpin);
PwmOut *L2Lpwm = new PwmOut(L2Lpin);
PwmOut *L2Hpwm = new PwmOut(L2Hpin);
PwmOut *L3Lpwm = new PwmOut(L3Lpin);
PwmOut *L3Hpwm = new PwmOut(L3Hpin);
// const float periodDC = 0.01;
// L1Lpwm->period(periodDC);
// L1Hpwm->period(periodDC);
// L2Lpwm->period(periodDC);
// L2Hpwm->period(periodDC);
// L3Lpwm->period(periodDC);
// L3Hpwm->period(periodDC);

// PwmOut L1Lpwm(L1Lpin);
// PwmOut L1Hpwm(L1Hpin);
// PwmOut L2Lpwm(L2Lpin);
// PwmOut L2Hpwm(L2Hpin);
// PwmOut L3Lpwm(L3Lpin);
// PwmOut L3Hpwm(L3Hpin);

// const float periodDC = 0.01f;

// L1Lpwm.period(periodDC);
// L1Hpwm.period(periodDC);
// L2Lpwm.period(periodDC);
// L2Hpwm.period(periodDC);
// L3Lpwm.period(periodDC);
// L3Hpwm.period(periodDC);


// QEI config
QEI wheel(CHA, CHB, NC, 117);

//Drive state to output table
const int8_t driveTable[] = {0x12, 0x18, 0x09, 0x21, 0x24, 0x06, 0x00, 0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07, 0x05, 0x03, 0x04, 0x01, 0x00, 0x02, 0x07};
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//@@@@@@@ Phase lead to make motor spin @@@@@@@ Must change according to PID output value
const int8_t lead = -2; //2 for forwards, -2 for backwards

//**********************************************
//*             Function Definitions           *
//**********************************************
//------- Convert photointerrupter inputs to a rotor state -------
inline int8_t readRotorState()
{
    return stateMap[I1 + 2 * I2 + 4 * I3];
}

//------- Modulus Function -------
float modulus(float n) {
    if(n<0) {
        n=-n;
        return n;
    } else {
        return n;
    }
}

//------- Run the motor with speed limitation -------
void motorOut(int8_t driveState){

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

    //Turn off first
    if (~driveOut & 0x01) *L1Lpwm = 0;
    if (~driveOut & 0x02) *L1Hpwm = 1;
    if (~driveOut & 0x04) *L2Lpwm = 0;
    if (~driveOut & 0x08) *L2Hpwm = 1;
    if (~driveOut & 0x10) *L3Lpwm = 0;
    if (~driveOut & 0x20) *L3Hpwm = 1;

    //Then turn on if speed is less than reference
    if (modulus(currentRPSValue) < Vref) {
        if (driveOut & 0x01) L1Lpwm->write(1-modulus(dutyCycle));
        if (driveOut & 0x02) L1Hpwm->write(modulus(dutyCycle));
        if (driveOut & 0x04) L2Lpwm->write(1-modulus(dutyCycle));
        if (driveOut & 0x08) L2Hpwm->write(modulus(dutyCycle));
        if (driveOut & 0x10) L3Lpwm->write(1-modulus(dutyCycle));
        if (driveOut & 0x20) L3Hpwm->write(modulus(dutyCycle));
        // if (driveOut & 0x01) L1Lpwm.write(modulus(dutyCycle));
        // if (driveOut & 0x02) L1Hpwm.write(1-modulus(dutyCycle));
        // if (driveOut & 0x04) L2Lpwm.write(modulus(dutyCycle));
        // if (driveOut & 0x08) L2Hpwm.write(1-modulus(dutyCycle));
        // if (driveOut & 0x10) L3Lpwm.write(modulus(dutyCycle));
        // if (driveOut & 0x20) L3Hpwm.write(1-modulus(dutyCycle));
    }
}

//------- Basic synchronisation routine -------
int8_t motorHome()
{
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    wait(1.0);
    //Get the rotor state
    return readRotorState();
}

//------- Drive motor using position sensed by PI -------
//dutyCycle global variable
void readPIrunMotor(){
    intState = readRotorState();
    if (intState != intStateOld) {
        intStateOld = intState;
        // we can move this into another function, this just tells me where the photointerrupter is in a 60 degree radius
        if (dutyCycle > 0) {
          motorOut((intState-orState+lead+6)%6); //For lead +ve Lead = 2 | +ve cw
        }
        if (dutyCycle < 0) { //-ve torque to slow down the spinning motor or reverse the direction
          motorOut((intState-orState-lead+6)%6); //For lead +ve Lead = 2 | -ve acw
        }
        //orstate is updated for re-syncing the rotor position every 2s in the main.cpp by interrupt
    }
}

//------- Speed from QEI and PI -------
void getRPSfromQEI(){
    lastPosition = currentPosition;
    currentPosition = wheel.getPulses();    //getPulses gets accumulated no. of pulses recorded
    float numberOfRevolutions = (currentPosition - lastPosition) / 117;
    currentRPSValue = (numberOfRevolutions / RPS_SAMPLING_RATE);
    pc.printf("QEI rps: %f \t", currentRPSValue);
    pc.printf("QEI count: %f \n\r", currentPosition);
}

//------- Controller -------
void controlInit() {
    controller.setInputLimits(0.0,targetPosition*100);
    controller.setOutputLimits(-1.0, 1.0); //Set duty cycle parameter as fraction
    controller.setBias(0.0);
    controller.setMode(AUTO); //SET MODE as auto
    controller.setSetPoint(targetPosition);
}

void controlR() {
    while(1) {
        controller.setProcessValue(currentPosition);
        dutyCycle = controller.compute();
        pc.printf("duty cycle: %f \n\r", dutyCycle);
        wait(PIDrate);
    }
}

//**********************************************
//             Main Function                   *
//**********************************************
int main()
{
    //******* Initialise the serial port *******

    pc.printf("Hello\n\r");

    //******* Run the motor synchronisation *******
    orState = motorHome();
    pc.printf("Rotor origin: %x\n\r", orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states

    //******* Setup interrupts to calculate speed from QEI and PI *******
    // speedTimer.start();
    // I1.rise(&getRPSfromPI);
    sampleRPS.attach(&getRPSfromQEI, RPS_SAMPLING_RATE);

    //******* Setup threads for controller *******
    controlInit();
    PIDthread.start(controlR);

    //******* Poll the rotor state and set the motor outputs accordingly to spin the motor *******
    while (1)
    {
        readPIrunMotor();
    }
}
