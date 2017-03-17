#include "mbed.h"
#include "rtos.h"
#include "implementation.h"
#include "PID.h"

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

PwmOut PID_PWM(PWM_ctrlpin); // check out PwnOut library for reading pulsewidth

//PID Controller
#define RATE 0.1
#define AUTO 1
int8_t R = 20;

PID controller(1.0, 0.0, 0.0, RATE);

// serial config
Serial pc(SERIAL_TX, SERIAL_RX);

// QEI config
QEI wheel(CHA, CHB, NC, 117);

void inputHandler()
{

    char input[256] = "";
    char c;
    int index = 0;

    do
    {
        c = pc.getc();
        input[index++] = c;
    } while (c != '/n');

    pc.printf("input is : %s", input);

    // IMPORTANT : I think semaphores should be used to wake up sleeping threads to do their tasks, investigate this more - THREAD SIGNAL WAIT
    // IMPORTANT : Threads should sleep after completing its task! - refer to thread state diagram
    // SLRE regex handlers, use capturing groups to get necessary data
    // if music command -> thread to musicHandler()
    // if rotation command -> thread to rotationHandler(), different types of rotation - R, V, R-V
    // threads for inputHandler(),rotationHandler() and musicHandler(), threads should sleep if not doing work - refer to thread state diagram
    // interrupts for sampling photointerrupter and position encoder information - provisionally DONE
}

void rotationHandler()
{
    //Update the process variable.
    controller.setProcessValue(wheel.getPulses());
    //Set the new output.
    co = controller.compute();
    //map co into PWM form control signal to drive motorout()
    if (co > 1.5)
      PID_PWM= 1;
    if (co < 1.5)
      PID_PWM= 0;
    // add here with the PWM read pulsewidth duration to energised the stator coils.
    
    //Wait for another loop calculation.
    wait(RATE);
}

void controlAlgorithm()
{
    // control theory!
    //Analog input from 0.0 to 3.3V
    controller.setInputLimits(0.0, R * 117.0);
    //Pwm output from 0.0 to 1.0
    controller.setOutputLimits(0.0, 3.0);
    //If there's a bias.
    controller.setBias(0);
    controller.setMode(AUTO);
    //We want the process variable to be 1.7V
    controller.setSetPoint(R * 117.0);
}

void musicHandler()
{
    // code to produce music, use a thread for this
}

//Set a given drive state
void motorOut(int8_t driveState)
{

    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];

    //Turn off first
    if (~driveOut & 0x01)
        L1L = 0;
    if (~driveOut & 0x02)
        L1H = 1;
    if (~driveOut & 0x04)
        L2L = 0;
    if (~driveOut & 0x08)
        L2H = 1;
    if (~driveOut & 0x10)
        L3L = 0;
    if (~driveOut & 0x20)
        L3H = 1;

    //Then turn on
    if (driveOut & 0x01)
        L1L = 1;
    if (driveOut & 0x02)
        L1H = 0;
    if (driveOut & 0x04)
        L2L = 1;
    if (driveOut & 0x08)
        L2H = 0;
    if (driveOut & 0x10)
        L3L = 1;
    if (driveOut & 0x20)
        L3H = 0;
}

int8_t orState = motorHome();
int8_t intState = 0;
int8_t intStateOld = 0;

// sample photointerrupter output via interrupt
void readPhotoInterrupterState()
{
    intState = readRotorState();
    if (intState != intStateOld)
    {
        intStateOld = intState;
        motorOut((intState - orState + lead + 6) % 6); //+6 to make sure the remainder is positive
    }
}

// we need to store 2 global states, lastPosition and currentPosition

const float RPM_SAMPLING_RATE = 0.1; // rate at which interrupt is called, also the time difference(t) since the last interrupt call
float lastPosition = 0;
float currentPosition = 0;
float currentRPMValue = 0;
// calls interrupt at sampling rate of samplingRateRPM -> calculate RPM with position encoder
void getRPMFromPositionEncoder()
{
    lastPosition = currentPosition;
    currentPosition = wheel.getPulses();
    float numberOfRevolutions = (currentPosition - lastPosition) / 117;
    currentRPMValue = (numberOfRevolutions / RPM_SAMPLING_RATE) * 60;
}

//Basic synchronisation routine
int8_t motorHome()
{
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    wait(1.0);

    //Get the rotor state
    return readRotorState();
}
