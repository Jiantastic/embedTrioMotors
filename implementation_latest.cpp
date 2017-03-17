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
****************initialisation of PID controller parameters****************
//float PIDrate = 0.1;
float Kp = 2.5; // try testing with low value
float Ki = 0.1; //do we even need this??
float Kd = 0.1; //to be iterated
// V10.1 setting
float Setpoint = 5.1; // from regex command input
float Input = 0; //
float Output = 0;
**************** we need to change these *******************
PwmOut pwm1(D1);
PwmOut pwm2(D0);
float threshold_rpm = 33; // this threshold is used to switch the coils between PWMout mode and DigitalOut mode

***************initialisation of PWM parameters*************
bool AUTOMATIC=1; // to enable the auto-tuning of PID algorithm
float PWM_period = 127;
float pulsewidth = 0; //initialisation of PID controller output, pulsewidth
float duty_cycle = 0; // can be +ve or -ve (-ve will trigger the sign change in phase lead to produce -ve torque)

PID myPID(&Input, &Output, &Setpoint, Kc, Ti, Td, DIRECT);
void controlAlgorithm(){
    // control theory!
    while(1){
        myPID.SetMode(AUTOMATIC);
        myPID.SetOutputLimits(-PWM_period,PWM_period);
        Input = currentRPMValue; // or currentPosition ? for 2nd order system
        myPID.Compute();
        pulsewidth = myPID.GetOutput();
        duty_cycle = pulsewidth / PWM_period;
        Thread::wait(1); //is this the rate at which the PID thread produces output? to be altered!
    }
}

void musicHandler(){
    // code to produce music, use a thread for this
}

//Set a given drive state
void motorOut(int8_t driveState){

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

  //pwm1.period(0.02);
  //pwm1.write(Output/255);
  //pwm2 = 1 - pwm1;

    if (currentRPMValue <= threshold_rpm) {
      //Since we cannot set the phase of the different PWM channels we cannot synchronise both the H and L pulses to be on at the same time.
      //So leave one set with digital inputs (L1H-L3H) as before and just have PWM on the other set (L1L-L3L).
      // do we perform all the changes in n-channel Mosfet concurrently in threads or sequentially as what's being done below?

      delete L1L; //default pin type declared as DigitalOut upon initialisation
      L1Lpwm = new PwmOut(L1L);

      delete L2L;
      L2Lpwm = new PwmOut(L2L);

      delete L3L;
      L3Lpwm = new PwmOut(L3L);

    }
    //Turn off first
    if (~driveOut & 0x01) L1Lpwm = 0;
    if (~driveOut & 0x02) L1H = 1;
    if (~driveOut & 0x04) L2Lpwm = 0;
    if (~driveOut & 0x08) L2H = 1;
    if (~driveOut & 0x10) L3Lpwm = 0;
    if (~driveOut & 0x20) L3H = 1;

    //Then turn on
    if (driveOut & 0x01) L1Lpwm ->write(duty_cycle);
    if (driveOut & 0x02) L1H = 0;
    if (driveOut & 0x04) L2Lpwm ->write(duty_cycle);
    if (driveOut & 0x08) L2H = 0;
    if (driveOut & 0x10) L3Lpwm ->write(duty_cycle);
    if (driveOut & 0x20) L3H = 0;

    else if (currentRPMValue > threshold_rpm) {
      // at high motor speed the dutycycle needs to be switched quickly but there is a latency whereby duty cycle is updated and applied upon the inductors.
      //so we use DigitalOut instead of PwmOut here
      //the PwmOut has higher priority over the DigitalOut but no disable function is available in the PwmOut library so:
      delete L1Lpwm;
      L1Ldigi = new DigitalOut(L1L);
      if (currentRPMValue >= SetPoint){
        L1Ldigi = 0;
      }
      else{
        L1Ldigi = 1;
      }

      delete L2Lpwm;
      L2Ldigi = new DigitalOut(L2L);
      if (currentRPMValue >= SetPoint){
        L2Ldigi = 0;
      }
      else{
        L2Ldigi = 1;
      }

      delete L3Lpwm;
      L3Ldigi = new DigitalOut(L3L);
      if (currentRPMValue >= SetPoint){
        L3Ldigi = 0;
      }
      else{
        L3Ldigi = 1;
      }
    }
    //Turn off first
    if (~driveOut & 0x01) L1Ldigi = 0;
    if (~driveOut & 0x02) L1H = 1;
    if (~driveOut & 0x04) L2Ldigi = 0;
    if (~driveOut & 0x08) L2H = 1;
    if (~driveOut & 0x10) L3Ldigi = 0;
    if (~driveOut & 0x20) L3H = 1;

    //Then turn on
    //[Attention] will shoot thru (short cct) occur with this configuration?
    if (driveOut & 0x01) L1Ldigi;
    if (driveOut & 0x02) L1H = 0;
    if (driveOut & 0x04) L2Ldigi;
    if (driveOut & 0x08) L2H = 0;
    if (driveOut & 0x10) L3Ldigi;
    if (driveOut & 0x20) L3H = 0;



}

int8_t orState = motorHome(); // how often do we need to re-sync the rotor ? eg N rotations?
int8_t intState = 0;
int8_t intStateOld = 0;

// sample photointerrupter output via interrupt
void readPhotoInterrupterState(duty_cycle){
    intState = readRotorState();
    if (intState != intStateOld) {
        intStateOld = intState;
        // we can move this into another function, this just tells me where the photointerrupter is in a 60 degree radius
        if (duty_cycle > 0) {
        motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
        }
        if (duty_cycle < 0) { //-ve torque to slow down the spinning motor or reverse the direction
        motorOut((intState-orState-lead+6)%6); //+6 to make sure the remainder is positive
        }
        //orstate is updated for re-syncing the rotor position every 2s in the main.cpp by interrupt

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
