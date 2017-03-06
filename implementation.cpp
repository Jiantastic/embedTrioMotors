#include "mbed.h"
#include "rtos.h"
#include "QEI.h"
#include "implementation.h"

void inputHandler(){
    // SLRE regex handlers, use capturing groups to get necessary data
    // if music command -> musicHandler()
    // if rotation command -> rotationHandler()
    // threads for inputHandler(),rotationHandler() and musicHandler()?
    // interrupts for sampling photointerrupter and position encoder information
}

void rotationHandler(){
    // code to produce rotation according to controlAlgorithm(), feedback = readPositionEncoderState()
    // calls photointerrupter data readRotorState() to modify motorOut()
}

void controlAlgorithm(){
    // control theory!
}

void musicHandler(){
    // code to produce music, use a thread for this
}

//Set a given drive state
void motorOut(int8_t driveState){
    
    //Lookup the output byte from the drive state.
    int8_t driveOut = driveTable[driveState & 0x07];
      
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
    
//Convert photointerrupter inputs to a rotor state, 1 2 and 4 and binary powers
inline int8_t readRotorState(){
    return stateMap[I1 + 2*I2 + 4*I3];
}

void readPositionEncoderState(){
    // QEI API 
}

//Basic synchronisation routine    
int8_t motorHome() {
    //Put the motor in drive state 0 and wait for it to stabilise
    motorOut(0);
    wait(1.0);
    
    //Get the rotor state
    return readRotorState();
}