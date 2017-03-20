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
const float Vref = 10;
const float Rref = 20;

const float thresholdRPS = 33;
const float periodDC = 0.01; //Set duty cycle period to 0.01s (10ms)

int8_t intState = 0;
int8_t intStateOld = 0;
int8_t orState = 0; //Rotor offset at motor state 0

const float RPS_SAMPLING_RATE = 0.1;      // rate at which interrupt is called, also the time difference(t) since the last interrupt call
float lastPosition = 0;
float currentPosition = 0;
float currentRPSValue = 0;                // global angular velocity/RPS value

float rotations=5; //Set a random starting variable - replaced by regex later on
float targetPos=117*Rref;
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

//Motor Drive outputs
DigitalOut L1Ldigi(L1Lpin);
DigitalOut L1Hdigi(L1Hpin);
DigitalOut L2Ldigi(L2Lpin);
DigitalOut L2Hdigi(L2Hpin);
DigitalOut L3Ldigi(L3Lpin);
DigitalOut L3Hdigi(L3Hpin);

// QEI config
QEI wheel(CHA, CHB, NC, 117);

//Drive state to output table
const int8_t driveTable[] = {0x12, 0x18, 0x09, 0x21, 0x24, 0x06, 0x00, 0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07, 0x05, 0x03, 0x04, 0x01, 0x00, 0x02, 0x07};
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//@@@@@@@ Phase lead to make motor spin @@@@@@@ Must change according to PID output value
const int8_t lead = 2; //2 for forwards, -2 for backwards

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

//------- Set a given drive state and moves motor -------
/*Brute force speed limitation
void motorOut(int8_t driveState){

    int8_t driveOut = driveTable[driveState & 0x07];

    //Turn off first
    if (~driveOut & 0x01) L1L = 0;
    if (~driveOut & 0x02) L1H = 1;
    if (~driveOut & 0x04) L2L = 0;
    if (~driveOut & 0x08) L2H = 1;
    if (~driveOut & 0x10) L3L = 0;
    if (~driveOut & 0x20) L3H = 1;

    //Turn on if current speed is less than reference
    if(currentRPSValue < Vref) {
        if (driveOut & 0x01) L1L = 1;
        if (driveOut & 0x02) L1H = 0;
        if (driveOut & 0x04) L2L = 1;
        if (driveOut & 0x08) L2H = 0;
        if (driveOut & 0x10) L3L = 1;
        if (driveOut & 0x20) L3H = 0;
    }
}
*/

void motorOut(int8_t driveState){

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

    //pwm1.period(0.02);
    //pwm1.write(Output/255);
    //pwm2 = 1 - pwm1;

    if (currentRPSValue <= thresholdRPS) { //low speed = PwmOut
        //Since we cannot set the phase of the different PWM channels we cannot synchronise both the H and L pulses to be on at the same time.
        //So leave one set with digital inputs (L1H-L3H) as before and just have PWM on the other set (L1L-L3L).
        // do we perform all the changes in n-channel Mosfet concurrently in threads or sequentially as what's being done below?
        delete L1Ldigi; //default pin type declared as DigitalOut upon initialisation
        L1Lpwm = new PwmOut(L1Lpin);
        L1Lpwm.period(periodDC);  //Period of pwm = 0.01seconds (10ms)
        delete L2Ldigi;
        L2Lpwm = new PwmOut(L2Lpin);
        L2Lpwm.period(periodDC);  //Period of pwm = 0.01seconds (10ms)        
        delete L3Ldigi;
        L3Lpwm = new PwmOut(L3Lpin);
        L3Lpwm.period(periodDC);  //Period of pwm = 0.01seconds (10ms)

        //Turn off first
        if (~driveOut & 0x01) L1Lpwm = 0;
        if (~driveOut & 0x02) L1Hdigi = 1;
        if (~driveOut & 0x04) L2Lpwm = 0;
        if (~driveOut & 0x08) L1Hdigi = 1;
        if (~driveOut & 0x10) L3Lpwm = 0;
        if (~driveOut & 0x20) L1Hdigi = 1;   

        //Then turn on if speed is less than reference
        if (currentRPSValue < Vref) {
            if (driveOut & 0x01) L1Lpwm.write(modulus(dutyCycle));
            if (driveOut & 0x02) L1Hdigi = 0;
            if (driveOut & 0x04) L2Lpwm.write(modulus(dutyCycle));
            if (driveOut & 0x08) L1Hdigi = 0;
            if (driveOut & 0x10) L3Lpwm.write(modulus(dutyCycle));
            if (driveOut & 0x20) L1Hdigi = 0;
        }
    }

    else { //high speed DigitalOut
        // at high motor speed the dutycycle needs to be switched quickly but there is a latency whereby duty cycle is updated and applied upon the inductors.
        // so we use DigitalOut instead of PwmOut here
        // the PwmOut has higher priority over the DigitalOut but no disable function is available in the PwmOut library so:
        delete L1Lpwm;
        L1Ldigi = new DigitalOut(L1Lpin);
        delete L2Lpwm;
        L2Ldigi = new DigitalOut(L2Lpin);
        delete L3Lpwm;
        L3Ldigi = new DigitalOut(L3Lpin);

        //Turn off first
        if (~driveOut & 0x01) L1Ldigi = 0;
        if (~driveOut & 0x02) L1Hdigi = 1;
        if (~driveOut & 0x04) L2Ldigi = 0;
        if (~driveOut & 0x08) L1Hdigi = 1;
        if (~driveOut & 0x10) L3Ldigi = 0;
        if (~driveOut & 0x20) L1Hdigi = 1;

        //Then turn on if current speed is less than reference
        //[Attention] will shoot thru (short cct) occur with this configuration?
        if(currentRPSValue < Vref) {
            if (driveOut & 0x01) L1Ldigi = 1;
            if (driveOut & 0x02) L1Hdigi = 0;
            if (driveOut & 0x04) L2Ldigi = 1;
            if (driveOut & 0x08) L1Hdigi = 0;
            if (driveOut & 0x10) L3Ldigi = 1;
            if (driveOut & 0x20) L1Hdigi = 0;
        }
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
}

//------- Controller -------
void controlInit() {
    controller.setInputLimits(0.0,targetPos); 
    controller.setOutputLimits(-1.0, 1.0); //Set duty cycle parameter as fraction
    controller.setBias(0.0);
    controller.setMode(AUTO); //SET MODE as auto
    controller.setSetPoint(targetPos); 
}

void controlR() {
    while(1) {
        controller.setProcessValue(currentPosition);
        dutyCycle = controller.compute(); 
        wait(PIDrate);
    } 
}

//**********************************************
//             Main Function                   *
//**********************************************
int main()
{
    //******* Initialise the serial port *******
    Serial pc(SERIAL_TX, SERIAL_RX);

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
