#include "mbed.h"
#include "rtos.h"
#include "QEI.h"
#include "implementation.h"
// NOTE : when to use inline for functions?
// use volatile for information accessed by multiple threads, mutex handler to prevent race conditions

// TODO
// 1. Interrupts instead of polling for photointerrupters - DONE, consider moving variable definition
// 2. Thread(semi-polling) for reading from Serial input - wait until new input comes in, keep processing until /n (enter) is found
// 3. Get QEI library working with pins - WORKING, more testing needed to determine accuracy - probably need to add a sync component with photointerrupters

Ticker samplePhotoInterrupter;
Ticker sampleRPM;
Thread control;
char input[255];
int charCount = 0;
//Main
// reading from serial input = main thread
int main() {

    // thread that handles the control algorithm inputs
    control.start(controlAlgorithm);
    
    //Run the motor synchronisation
    pc.printf("Rotor origin: %x\n\r",orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states
    
    //Interrupt to get rotor state and set the motor outputs accordingly to spin the motor
    // samplePhotoInterrupter.attach(&readPhotoInterrupterState,0.001);
    samplePhotoInterrupter.attach(&readPhotoInterrupterState,Output/255);
    sampleRPM.attach(&getRPMFromPositionEncoder,RPM_SAMPLING_RATE);
    
    while (1) {
        if(pc.readable()){
            do{
                ch = pc.getc();
                pc.putc(ch);
                input[charCount++] = ch;
            }while(ch != 10 && ch != 13);

            // TODO: testing
            pc.putc('\n');
            pc.putc('\r');

            // SLRE regular expression handler here - to insert code after extensive testing
            // we call different controllers(settings or otherwise) depending on speed (fast/slow) and type (R,V,R-V)

            pc.printf("Current RPM speed value is: %f\n", currentRPMValue);
            wait(0.5);
        }
    }
}


