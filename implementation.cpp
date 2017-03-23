#include "mbed.h"
#include "rtos.h"
#include "implementation.h"

//**********************************************
//*             Variable Definitions           *
//**********************************************
RawSerial pc(SERIAL_TX, SERIAL_RX);

int8_t intState = 0;
int8_t intStateOld = 0;
int8_t orState = 0;             //Rotor offset at motor state 0

float lastPosition = 0;
float currentPosition = 0;
float currentRPSValue = 0;                // global angular velocity/RPS value

//float rotations=5; //Set a random starting variable - replaced by regex later on
float targetPosition=117*Rref;  //30*117=3510 100*117=11700
float PIDrate = 0.2;
float Kc = 5.0;
float Ti = 0.0;
float Td = 0.0;
float speedControl = 0;
float dutyCycle = 0; //global duty cycle to be passed to motorOut
bool AUTO = 1;
PID controller(Kc, Ti, Td, PIDrate);
Thread PIDthread;

float Vref = 0;       // speed to be changed by regex
float Rref = 0;       // rotation to be changed by regex

//@@@@@@@ Photointerrupter inputs @@@@@@@
// DigitalIn I1(I1pin);
//Define I1 as interrupt inputs
DigitalIn I1(I1pin);
DigitalIn I2(I2pin);
DigitalIn I3(I3pin);

Ticker sampleRPS;
Ticker PrintRPS;
//Timer speedTimer;

//@@@@@@@ QEI inputs @@@@@@@ NOT USED
DigitalIn CHAInput(CHA);
DigitalIn CHBInput(CHB);


//Motor Drive outputs
DigitalOut *L1Ldigi = new DigitalOut(L1Lpin);
DigitalOut *L1Hdigi = new DigitalOut(L1Hpin);
DigitalOut *L2Ldigi = new DigitalOut(L2Lpin);
DigitalOut *L2Hdigi = new DigitalOut(L2Hpin);
DigitalOut *L3Ldigi = new DigitalOut(L3Lpin);
DigitalOut *L3Hdigi = new DigitalOut(L3Hpin);

PwmOut *L1Lpwm = new PwmOut(L1Lpin);
PwmOut *L1Hpwm = new PwmOut(L1Hpin);
PwmOut *L2Lpwm = new PwmOut(L2Lpin);
PwmOut *L2Hpwm = new PwmOut(L2Hpin);
PwmOut *L3Lpwm = new PwmOut(L3Lpin);
PwmOut *L3Hpwm = new PwmOut(L3Hpin);

// QEI config
QEI wheel(CHA, CHB, NC, 117);





//**********************************************
//*             Function Definitions           *
//**********************************************

//------- Modulus Function -------
float modulus(float n) {
    if(n<0) {
        return -n;
    }
    return n;
}

//------- Run the motor with speed limitation -------
void motorOut(int8_t driveState){

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

    //Turn off first
    if (currentRPSValue <= thresholdRPS) {

        // if PWM pointers point to nothing, create new PwmOut Pins
        if(L1Lpwm == NULL){
            L1Lpwm = new PwmOut(L1Lpin);
            L1Hpwm = new PwmOut(L1Hpin);
            L2Lpwm = new PwmOut(L2Lpin);
            L2Hpwm = new PwmOut(L2Hpin);
            L3Lpwm = new PwmOut(L3Lpin);
            L3Hpwm = new PwmOut(L3Hpin);
        }

        // if digital pin pointers point to something, delete them

        if(L1Ldigi != NULL){
            // deallocate DigitalPin pointers memory
            delete L1Ldigi;
            delete L1Hdigi;
            delete L2Ldigi;
            delete L2Hdigi;
            delete L3Ldigi;
            delete L3Hdigi;

            // set DigitalPin pointers to NULL
            L1Ldigi = NULL;
            L1Hdigi = NULL;
            L2Ldigi = NULL;
            L2Hdigi = NULL;
            L3Ldigi = NULL;
            L3Hdigi = NULL;
        }

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
        }
    }
    else { //high speed DigitalOut

        // if Digital Pins point to nothing, create digital pins
        if(L1Ldigi == NULL){
            L1Ldigi = new DigitalOut(L1Lpin);
            L1Hdigi = new DigitalOut(L1Hpin);
            L2Ldigi = new DigitalOut(L2Lpin);
            L2Hdigi = new DigitalOut(L2Hpin);
            L3Ldigi = new DigitalOut(L3Lpin);
            L3Hdigi = new DigitalOut(L3Hpin);
        }

        // if PWM pin pointers still exist, then delete them
        if(L1Lpwm != NULL){
            // deallocate PWM pointers memory
            delete L1Lpwm;
            delete L1Hpwm;
            delete L2Lpwm;
            delete L2Hpwm;
            delete L3Lpwm;
            delete L3Hpwm;

            // set PWM pointers to NULL
            L1Lpwm = NULL;
            L1Hpwm = NULL;
            L2Lpwm = NULL;
            L2Hpwm = NULL;
            L3Lpwm = NULL;
            L3Hpwm = NULL;
        }


        //Turn off first
        if (~driveOut & 0x01) *L1Ldigi = 0;
        if (~driveOut & 0x02) *L1Hdigi = 1;
        if (~driveOut & 0x04) *L2Ldigi = 0;
        if (~driveOut & 0x08) *L1Hdigi = 1;
        if (~driveOut & 0x10) *L3Ldigi = 0;
        if (~driveOut & 0x20) *L1Hdigi = 1;

        //Then turn on if current speed is less than reference
        //[Attention] will shoot thru (short cct) occur with this configuration?
        if(currentRPSValue < Vref) {
            if (driveOut & 0x01) *L1Ldigi = 1;
            if (driveOut & 0x02) *L1Hdigi = 0;
            if (driveOut & 0x04) *L2Ldigi = 1;
            if (driveOut & 0x08) *L2Hdigi = 0;
            if (driveOut & 0x10) *L3Ldigi = 1;
            if (driveOut & 0x20) *L3Hdigi = 0;
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

//------- Speed from QEI and PI -------
void getRPSfromQEI(){
    lastPosition = currentPosition;
    currentPosition = wheel.getPulses();    //getPulses gets accumulated no. of pulses recorded
    float numberOfRevolutions = (currentPosition - lastPosition) / 117;
    currentRPSValue = (numberOfRevolutions / RPS_SAMPLING_RATE);
}

//------- Controller -------
void controlInit() {
    controller.setInputLimits(0.0,targetPosition*50);
    controller.setOutputLimits(-1.0, 1.0); //Set duty cycle parameter as fraction
    controller.setBias(0.0);
    controller.setMode(AUTO); //SET MODE as auto
    controller.setSetPoint(targetPosition);
}

//void controlR() {
////    pc.printf("control init");
//    while(1) {
//        controller.setProcessValue(currentPosition);
//        dutyCycle = controller.compute();
//            intState = readRotorState();
//    if (intState != intStateOld) {
//        intStateOld = intState;
//        // we can move this into another function, this just tells me where the photointerrupter is in a 60 degree radius
//        if (dutyCycle > 0) {
//          motorOut((intState-orState+lead+6)%6); //For lead +ve Lead = 2 | +ve cw
//        }
//        if (dutyCycle < 0) { //-ve torque to slow down the spinning motor or reverse the direction
//          motorOut((intState-orState-lead+6)%6); //For lead +ve Lead = 2 | -ve acw
//        }
//        //orstate is updated for re-syncing the rotor position every 2s in the main.cpp by interrupt
//    }
//        Thread::wait(PIDrate);
//    }
//}

void printRPSfromQEI(){
      pc.printf("QEI rps: %f \t", currentRPSValue);
      pc.printf("QEI count: %f \n\r", currentPosition);

}


//------- Drive motor using position sensed by PI -------
//dutyCycle global variable
// TODO : Make this a thread
//void readPIrunMotor(){
//    // pc.printf("QEI rps: %f \t", currentRPSValue);
//    // pc.printf("QEI count: %f \n\r", currentPosition);
//    intState = readRotorState();
//    if (intState != intStateOld) {
//        intStateOld = intState;
//        // we can move this into another function, this just tells me where the photointerrupter is in a 60 degree radius
//        if (dutyCycle > 0) {
//          motorOut((intState-orState+lead+6)%6); //For lead +ve Lead = 2 | +ve cw
//        }
//        if (dutyCycle < 0) { //-ve torque to slow down the spinning motor or reverse the direction
//          motorOut((intState-orState-lead+6)%6); //For lead +ve Lead = 2 | -ve acw
//        }
//        //orstate is updated for re-syncing the rotor position every 2s in the main.cpp by interrupt
//    }
//}






void readPIrunMotor(){
   // pc.printf("QEI rps: %f \t", currentRPSValue);
   // pc.printf("QEI count: %f \n\r", currentPosition);
   while(1){
   intState = readRotorState();
   if (intState != intStateOld) {
       intStateOld = intState;
       // we can move this into another function, this just tells me where the photointerrupter is in a 60 degree radius
        motorOut((intState-orState+lead+6)%6); //For lead +ve Lead = 2 | +ve cw

       //orstate is updated for re-syncing the rotor position every 2s in the main.cpp by interrupt
   }
   Thread::wait(5);
   }
}