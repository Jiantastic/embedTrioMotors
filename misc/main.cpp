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

    pc.printf("Hello\n\r");

    //******* Run the motor synchronisation *******
    orState = motorHome();
    pc.printf("Rotor origin: %x\n\r", orState);
    //orState is subtracted from future rotor state inputs to align rotor and motor states

    //******* Setup interrupts to calculate speed from QEI and PI *******
    // speedTimer.start();
    // I1.rise(&getRPSfromPI);
    sampleRPS.attach(&getRPSfromQEI, RPS_SAMPLING_RATE);
    PrintRPS.attach(&printRPSfromQEI, 10);
    //******* Setup threads for controller *******
//    controlInit();
    
    

    //******* Poll the rotor state and set the motor outputs accordingly to spin the motor *******
    while (1)
    {
        // REGEX HANDLER
        if(pc.readable()){
            // extensive testing is needed to see if memset does what it is supposed to do
            // TODO : have not tested with interrupts + other threads
            memset(input, 0, 255);
            int charCount = 0;
            do{
                ch = pc.getc();
                pc.putc(ch);
                input[charCount++] = ch;
            } while(ch != 10 && ch != 13);

            const char *request = (const char*)input;

            pc.printf("My Name is %s", request);

            // TODO: testing, did I have to to add [\n\r]+ because of this?
            pc.putc('\n');
            pc.putc('\r');

            // SLRE regular expression handler here - to insert code after extensive testing
            // we call different controllers(settings or otherwise) depending on speed (fast/slow) and type (R,V,R-V)

            if(slre_match("^R(-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)[\r\n]+$",
                request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 1 - R command ONLY\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr);

                float ftemp = atof(caps[0].ptr);
                Vref = 35;
                Rref = ftemp;
                

                // start thread controller
            
            }
            else if(slre_match("^V([0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){
                        
                pc.printf("Group 2 - V command ONLY\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr);
                
                float ftemp = atof(caps[0].ptr);
                Vref = ftemp;
                
                pc.printf("hello world here");
                PIDthread.start(readPIrunMotor);
                
//                for(int i=0;i<5;i++){
//                    intState = readRotorState();
//                    if (intState != intStateOld) {
//                        intStateOld = intState;
//                        initMotorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
//                    }
//                }
                
                // start thread controller
                
            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?)V([0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 3 - RV command, NO decimals\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr);
            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?\\.[0-9][0-9]?)V([0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 4 - RV command, decimal for R ONLY\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr);

            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?)V([0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                pc.printf("Group 5 - RV command, decimal for V ONLY\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr);

            }
            else if(slre_match("^R(-?[0-9][0-9]?[0-9]?\\.[0-9][0-9]?)V([0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?)[\r\n]+$",
                    request, strlen(request), caps, 10, 0) > 0){

                
                pc.printf("Group 6 - RV command, decimal for R AND V\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr);

            }
            else if(slre_match("^T([A-G][#\\^]?[1-8])([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?[\r\n]+$",
                    request, strlen(request), caps, 16, 0) > 0){

                pc.printf("Group 7 - Music!\n");

                pc.printf("CAP 1: [%.*s], CAP 2: [%.*s], CAP 3: [%.*s], CAP 4: [%.*s], CAP 5: [%.*s], CAP 6: [%.*s], CAP 7: [%.*s], CAP 8: [%.*s], CAP 9: [%.*s], CAP 10: [%.*s], CAP 11: [%.*s], CAP 12: [%.*s], CAP 13: [%.*s], CAP 14: [%.*s], CAP 15: [%.*s], CAP 16: [%.*s]\n",
                caps[0].len, caps[0].ptr,
                caps[1].len, caps[1].ptr,
                caps[2].len, caps[2].ptr,
                caps[3].len, caps[3].ptr,
                caps[4].len, caps[4].ptr,
                caps[5].len, caps[5].ptr,
                caps[6].len, caps[6].ptr,
                caps[7].len, caps[7].ptr,
                caps[8].len, caps[8].ptr,
                caps[9].len, caps[9].ptr,
                caps[10].len, caps[10].ptr,
                caps[11].len, caps[11].ptr,
                caps[12].len, caps[12].ptr,
                caps[13].len, caps[13].ptr,
                caps[14].len, caps[14].ptr,
                caps[15].len, caps[15].ptr);
                
            }
            else{
                pc.printf("Error parsing. Please enter a valid command.");
            }
            Thread::wait(5);
        }
    }
}
