#include "mbed.h"
#include "rtos.h"
#include "implementation.h"

//Status LED
DigitalOut led1(LED1);

//Photointerrupter inputs
DigitalIn I1(I1pin);
DigitalIn I2(I2pin);
DigitalIn I3(I3pin);


DigitalOut CHAInput(CHA);
DigitalOut CHBInput(CHB);

//Motor Drive outputs
DigitalOut L1L(L1Lpin);
DigitalOut L1H(L1Hpin);
DigitalOut L2L(L2Lpin);
DigitalOut L2H(L2Hpin);
DigitalOut L3L(L3Lpin);
DigitalOut L3H(L3Hpin);

// serial config
RawSerial pc(SERIAL_TX, SERIAL_RX);

// QEI config
QEI wheel (CHA, CHB, NC, 117);

void rotationHandler(){
    // code to produce rotation according to controlAlgorithm(), feedback = readPositionEncoderState()
    // calls photointerrupter data readRotorState() to modify motorOut()
}

// thread that continuously updates rate at which windings are changed (in the form of interrupts)

float PIDrate = 0.1;
float Kc = 2.5;
float Ti = 0.1;
float Td = 0.1;
// V10.1 setting
float Setpoint = 5.1;
float Input = 0;
float Output = 0;
PwmOut pwm1(D1);
PwmOut pwm2(D0);


PID myPID(&Input, &Output, &Setpoint, Kc, Ti, Td, DIRECT);
void controlAlgorithm(){
    // control theory!
    while(1){
        myPID.SetMode(AUTOMATIC);
        Input = currentRPMValue;
        myPID.Compute();
        Thread::wait(1);
    }
}

void musicHandler(){
    // code to produce music, use a thread for this
}

//Set a given drive state
void motorOut(int8_t driveState){

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

    pwm1.period(0.02);
    pwm1.write(Output/255);
    pwm2 = 1 - pwm1;

    //Turn off first
    if (~driveOut & 0x01) L1L = 0;
    if (~driveOut & 0x02) L1H = 1;
    if (~driveOut & 0x04) L2L = 0;
    if (~driveOut & 0x08) L2H = 1;
    if (~driveOut & 0x10) L3L = 0;
    if (~driveOut & 0x20) L3H = 1;

    //Then turn on
    if (driveOut & 0x01) L1L = 1;
    if (driveOut & 0x02) L1H = 0;
    if (driveOut & 0x04) L2L = 1;
    if (driveOut & 0x08) L2H = 0;
    if (driveOut & 0x10) L3L = 1;
    if (driveOut & 0x20) L3H = 0;
}

int8_t orState = motorHome();
int8_t intState = 0;
int8_t intStateOld = 0;

// sample photointerrupter output via interrupt
void readPhotoInterrupterState(){
    intState = readRotorState();
    if (intState != intStateOld) {
        intStateOld = intState;
        // we can move this into another function, this just tells me where the photointerrupter is in a 60 degree radius
        motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
    }
}


// we need to store 2 global states, lastPosition and currentPosition

const float RPM_SAMPLING_RATE = 0.1;      // rate at which interrupt is called, also the time difference(t) since the last interrupt call
float lastPosition = 0;
float currentPosition = 0;
float currentRPMValue = 0;                // global angular velocity/RPM value
// calls interrupt at sampling rate of samplingRateRPM -> calculate RPM with position encoder
void getRPMFromPositionEncoder(){
    lastPosition = currentPosition;
    currentPosition = wheel.getPulses();
    float numberOfRevolutions = (currentPosition - lastPosition) / 117;
    currentRPMValue = (numberOfRevolutions / RPM_SAMPLING_RATE);
}

//Basic synchronisation routine
int8_t motorHome() {
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    wait(1.0);

    //Get the rotor state
    return readRotorState();
}
