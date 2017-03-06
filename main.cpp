#include "mbed.h"
#include "rtos.h"
#include "QEI.h"
#include "implementation.h"

// NOTE : when to use inline for functions?
// use volatile for information accessed by multiple threads, mutex handler to prevent race conditions
    
//Main
int main() {
    int8_t orState = 0;    //Rotot offset at motor state 0
    
    //Initialise the serial port
    Serial pc(SERIAL_TX, SERIAL_RX);
    int8_t intState = 0;
    int8_t intStateOld = 0;
    pc.printf("Hello\n\r");
    
    //Run the motor synchronisation
    orState = motorHome();
    pc.printf("Rotor origin: %x\n\r",orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    
    //Poll the rotor state and set the motor outputs accordingly to spin the motor
    while (1) {
        intState = readRotorState();
        if (intState != intStateOld) {
            intStateOld = intState;
            motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
        }
    }
}

