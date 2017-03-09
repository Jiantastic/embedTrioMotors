#include "mbed.h"
#include "rtos.h"
#include "QEI.h"
#include "implementation.h"
#include "PID.h"
// NOTE : when to use inline for functions?
// use volatile for information accessed by multiple threads, mutex handler to prevent race conditions

// TODO
// 1. Interrupts instead of polling for photointerrupters - DONE, consider moving variable definition
// 2. Thread(semi-polling) for reading from Serial input - wait until new input comes in, keep processing until /n (enter) is found
// 3. Get QEI library working with pins - WORKING, more testing needed to determine accuracy - probably need to add a sync component with photointerrupters

Ticker samplePhotoInterrupter;
QEI wheel (CHA, CHB, NC, 117);

//Main
int main() {

    pc.printf("Hello\n\r");
    
    //Run the motor synchronisation
    pc.printf("Rotor origin: %x\n\r",orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    
    //Interrupt to get rotor state and set the motor outputs accordingly to spin the motor
    samplePhotoInterrupter.attach(&readPhotoInterrupterState,0.001);
    
    while (1) {
        pc.printf("No Revolutions is: %i\n", wheel.getPulses()/117 );
    }
    
    
}


