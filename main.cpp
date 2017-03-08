#include "mbed.h"
#include "rtos.h"
#include "implementation.h"

// NOTE : when to use inline for functions?
// use volatile for information accessed by multiple threads, mutex handler to prevent race conditions

// TODO
// 1. Interrupts instead of polling for photointerrupters
// 2. Thread(semi-polling)/Interrupt for reading from Serial input

Ticker samplePhotoInterrupter;

//Main
int main() {
    
    orState = 0;
    intState = 0;
    intStateOld = 0;

    pc.printf("Hello\n\r");
    
    //Run the motor synchronisation
    orState = motorHome();
    pc.printf("Rotor origin: %x\n\r",orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    
    //Poll the rotor state and set the motor outputs accordingly to spin the motor
    samplePhotoInterrupter.attach(&readPhotoInterrupterState,0.001);
    while (1) {
        pc.printf("interrupt is getting called");
    }
}


