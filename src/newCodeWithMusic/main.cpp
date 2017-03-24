#include "mbed.h"
#include "rtos.h"
#include "PID.h"
#include "QEI.h"
#include "implementation.h"
#include "slre.h"
#include <cstring>
#include <cstdlib>

//**********************************************
//             Main Function                   *
//**********************************************

char input[255];
char ch;
struct slre_cap caps[16];

int main()
{
    //******* Initialise the serial port *******

    pc.printf("Hello, please wait for rotor to initialise\n\r");

    //******* Run the motor synchronisation *******
    orState = motorHome();
    pc.printf("Rotor origin: %x\n\r", orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states

    //******* Setup interrupts to calculate speed from QEI and PI *******
    // speedTimer.start();
    // I1.rise(&getRPSfromPI);
    sampleRPS.attach(&getRPSfromQEI, RPS_SAMPLING_RATE);
    PrintRPS.attach(&printRPSfromQEI, 10);


    L1Lpwm.period_ms(2);
    L2Lpwm.period_ms(2);
    L3Lpwm.period_ms(2);

    //******* Setup threads for controller *******
//    controlInit();
    
    //******* Poll the rotor state and set the motor outputs accordingly to spin the motor *******
    while (1)
    {
        // REGEX HANDLER
        if(pc.readable()){
            memset(input, 0, 255);
            int charCount = 0;
            do{
                ch = pc.getc();
                pc.putc(ch);
                input[charCount++] = ch;
            } while(ch != 10 && ch != 13);

            const char *request = (const char*)input;

            // TODO: testing, did I have to to add [\n\r]+ because of this?
            pc.putc('\n');
            pc.putc('\r');

            // SLRE regular expression handler here - to insert code after extensive testing
            // we call different controllers(settings or otherwise) depending on speed (fast/slow) and type (R,V,R-V)

            if(slre_match("^R(-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)[\r\n]+$",
                request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 1 - R command ONLY\n\r");

                wheel.reset();

                // set veloctiy and rotation values
                float rCommandToFloat = atof(caps[0].ptr);
                Vref = 25;      // temp max speed
                Rref = rCommandToFloat;
                targetPosition = 117*Rref;
                

                // start controller thread
                controlInit();
                pidThread.start(controlR);
            
            }
            else if(slre_match("^V([0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){
                        
                pc.printf("Group 2 - V command ONLY\n\r");

                wheel.reset();

                float vCommandToFloat = atof(caps[0].ptr);
                Vref = vCommandToFloat;
                Rref = 99999;
       
                // start controller thread
                controlInit();
                pidThread.start(controlR);
                
            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?)V([0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 3 - RV command, NO decimals\n\r");

                wheel.reset();

                float rCommandToFloat = atof(caps[0].ptr);
                float vCommandToFloat = atof(caps[1].ptr);

                Rref = rCommandToFloat;
                Vref = vCommandToFloat;

                // start controller thread
                controlInit();
                pidThread.start(controlR);
                

            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?\\.[0-9][0-9]?)V([0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 4 - RV command, decimal for R ONLY\n\r");

                wheel.reset();

                float rCommandToFloat = atof(caps[0].ptr);
                float vCommandToFloat = atof(caps[1].ptr);

                Rref = rCommandToFloat;
                Vref = vCommandToFloat;

                // start controller thread
                controlInit();
                pidThread.start(controlR);

            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?)V([0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 5 - RV command, decimal for V ONLY\n\r");

                wheel.reset();

                float rCommandToFloat = atof(caps[0].ptr);
                float vCommandToFloat = atof(caps[1].ptr);

                Rref = rCommandToFloat;
                Vref = vCommandToFloat;

                // start controller thread
                controlInit();
                pidThread.start(controlR);

            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?\\.[0-9][0-9]?)V([0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                
                pc.printf("Group 6 - RV command, decimal for R AND V\n\r");

                wheel.reset();

                float rCommandToFloat = atof(caps[0].ptr);
                float vCommandToFloat = atof(caps[1].ptr);

                Rref = rCommandToFloat;
                Vref = vCommandToFloat;

                // start controller thread
                controlInit();
                pidThread.start(controlR);

            }
            else if(slre_match("^T([A-G][#\\^]?[1-8])([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?[\r\n]+$",
                    request, strlen(request), caps, 16, 0) > 0){

                pc.printf("Group 7 - Music!\n\r");

                while(1){
                    L2Hdigi = 0;
                    for(int i=0; i<15; i++) {
                        if(caps[i].ptr != NULL) {
                            playMusic(caps[i].ptr,caps[i].len);
                            pc.printf("Waiting for next note \n\r");
                        }
                    }
                }

            }
            else{
                pc.printf("Error parsing. Please enter a valid command.\n\r");
            }
            Thread::wait(0.1);
        }
    }
}
