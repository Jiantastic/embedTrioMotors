#include "mbed.h"
#include "rtos.h"
#include "implementation.h"

//**********************************************
//*             Variable Definitions           *
//**********************************************

DigitalIn I1(I1pin);
DigitalIn I2(I2pin);
DigitalIn I3(I3pin);

//@@@@@@@ QEI inputs @@@@@@@ NOT USED
DigitalIn CHAInput(CHA);
DigitalIn CHBInput(CHB);


//Motor Drive outputs
DigitalOut *L1Ldigi = new DigitalOut(L1Lpin);
DigitalOut *L1Hdigi = new DigitalOut(L1Hpin);
DigitalOut *L2Ldigi = new DigitalOut(L2Lpin);
DigitalOut *L2Hdigi = new DigitalOut(L2Hpin);
DigitalOut *L3Ldigi = new DigitalOut(L3Lpin);
DigitalOut *L3Hdigi = new DigitalOut(L3Hpin);

PwmOut *L1Lpwm = new PwmOut(L1Lpin);
PwmOut *L2Lpwm = new PwmOut(L2Lpin);
PwmOut *L3Lpwm = new PwmOut(L3Lpin);

RawSerial pc(SERIAL_TX, SERIAL_RX);
// QEI config
QEI wheel(CHA, CHB, NC, 117);

int8_t intState = 0;
int8_t intStateOld = 0;
int8_t orState = 0;

float lastPosition = 0;
float currentPosition = 0;
float currentRPSValue = 0;               // global angular velocity/RPS value

float targetPosition=117*Rref;
float PIDrate = 0.2;

float Kc = 20.0;
float Ti = 0.125;
float Td = 4.0;
bool AUTO = 1;
float dutyCycle = 1;                    //global duty cycle to be passed to motorOut
float oriDutyCycle = 0;
float tmpdutycycle = 0;
PID controller(Kc, Ti, Td, PIDrate);
Thread pidThread;

Ticker sampleRPS;
Ticker PrintRPS;

float Vref = 25;
float Rref = 0;


//**********************************************
//*             Function Definitions           *
//**********************************************

//------- Modulus Function -------
float modulus(float n) {
    if(n<0) {
        return -n;
    }
    return n;
}

//------- Run the motor with speed limitation -------
void motorOut(int8_t driveState){

    int8_t driveOut = driveTable[driveState & 0x07];

    if (currentRPSValue <= thresholdRPS) {      //Use PWM at low speeds
        if(L1Lpwm == NULL){                 // if PWM pointers point to nothing, create new PwmOut Pins
            L1Lpwm = new PwmOut(L1Lpin);
            L2Lpwm = new PwmOut(L2Lpin);
            L3Lpwm = new PwmOut(L3Lpin);
        }

        if(L1Ldigi != NULL){            // if digital pin pointers point to something, delete them
            delete L1Ldigi;
            delete L2Ldigi;;
            delete L3Ldigi;
            L1Ldigi = NULL;
            L2Ldigi = NULL;
            L3Ldigi = NULL;
        }

        if (~driveOut & 0x01) *L1Lpwm = 0;
        if (~driveOut & 0x02) *L1Hdigi = 1;
        if (~driveOut & 0x04) *L2Lpwm = 0;
        if (~driveOut & 0x08) *L2Hdigi = 1;
        if (~driveOut & 0x10) *L3Lpwm = 0;
        if (~driveOut & 0x20) *L3Hdigi = 1;

        if (currentRPSValue < Vref) {
            if (driveOut & 0x01) L1Lpwm->write(dutyCycle);
            if (driveOut & 0x02) *L1Hdigi = 0;
            if (driveOut & 0x04) L2Lpwm->write(dutyCycle);
            if (driveOut & 0x08) *L2Hdigi = 0;
            if (driveOut & 0x10) L3Lpwm->write(dutyCycle);
            if (driveOut & 0x20) *L3Hdigi = 0;
        }
    }

    else { //High Speed use digitalpin
        if(L1Ldigi == NULL){
            L1Ldigi = new DigitalOut(L1Lpin);
            L2Ldigi = new DigitalOut(L2Lpin);
            L3Ldigi = new DigitalOut(L3Lpin);
        }
        if(L1Lpwm != NULL){
            delete L1Lpwm;
            delete L2Lpwm;
            delete L3Lpwm;
            L1Lpwm = NULL;
            L2Lpwm = NULL;
            L3Lpwm = NULL;
        }

        if (~driveOut & 0x01) *L1Ldigi = 0;
        if (~driveOut & 0x02) *L1Hdigi = 1;
        if (~driveOut & 0x04) *L2Ldigi = 0;
        if (~driveOut & 0x08) *L1Hdigi = 1;
        if (~driveOut & 0x10) *L3Ldigi = 0;
        if (~driveOut & 0x20) *L1Hdigi = 1;

        if(currentRPSValue < Vref) {
            if (driveOut & 0x01) *L1Ldigi = 1;
            if (driveOut & 0x02) *L1Hdigi = 0;
            if (driveOut & 0x04) *L2Ldigi = 1;
            if (driveOut & 0x08) *L2Hdigi = 0;
            if (driveOut & 0x10) *L3Ldigi = 1;
            if (driveOut & 0x20) *L3Hdigi = 0;
        }
    }
}

//------- Basic synchronisation routine -------
int8_t motorHome()
{
    motorOut(0);
    wait(3.0);
    return readRotorState();
}


//------- Speed from QEI and PI -------
void getRPSfromQEI(){
    lastPosition = currentPosition;
    currentPosition = wheel.getPulses();    //getPulses gets accumulated no. of pulses recorded
    float numberOfRevolutions = (currentPosition - lastPosition) / 117;
    currentRPSValue = modulus((numberOfRevolutions / RPS_SAMPLING_RATE));
}

//------- Controller -------
void controlInit() {
    controller.setInputLimits(0.0,targetPosition*10);
    controller.setOutputLimits(-1.0, 1.0);
    controller.setBias(0.0);
    controller.setMode(AUTO);
    controller.setSetPoint(targetPosition);
    // pc.printf("Kc: %f\n",controller.getPParam());
    // pc.printf("Ki: %f\n",controller.getIParam());
    // pc.printf("Kd: %f\n",controller.getDParam());
}

void controlR() {
    while(1) {
        controller.setProcessValue(modulus(currentPosition));
        oriDutyCycle = controller.compute();
        readPIrunMotor();
        Thread::wait(PIDrate);
    }
}

void printRPSfromQEI(){
      pc.printf("QEI rps: %f \t", currentRPSValue);
      pc.printf("QEI count: %f \n\r", currentPosition);
      pc.printf("duty cycle: %f \n\r", oriDutyCycle);
}



float music[12] = {440.00,466.16,493.88,523.25,554.37,587.33,622.25,659.25,698.46,739.99,783.99,830.61};

void playMusic(const char* c, int l) {                          //Take in i
    pc.printf("playing music \n\r");
    switch ( c[0] ) {  
         case 'A':  
            switch(c[1]) {
                case '#':
                    L1Lpwm->period(1.0f/music[1]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                case '^' :
                    L1Lpwm->period(1.0f/music[11]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[0]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            }         
         case 'B':  
            switch(c[1]) {
                case '^' :
                    L1Lpwm->period(1.0f/music[1]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[2]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            }  
        case 'C':  
            switch(c[1]) {
                case '#':
                    L1Lpwm->period(1.0f/music[4]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[3]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            } 
        case 'D':  
            switch(c[1]) {
                case '#':
                    L1Lpwm->period(1.0f/music[6]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                case '^' :
                    L1Lpwm->period(1.0f/music[4]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[5]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            } 
        case 'E':  
            switch(c[1]) {
                case '^' :
                    L1Lpwm->period(1.0f/music[6]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[7]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            } 
        case 'F':  
            switch(c[1]) {
                case '#':
                    L1Lpwm->period(1.0f/music[9]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[8]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            } 
        case 'G':  
            switch(c[1]) {
                case '#':
                    L1Lpwm->period(1.0f/music[11]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                case '^' :
                    L1Lpwm->period(1.0f/music[9]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[2]-48);
                    break;
                default:
                    L1Lpwm->period(1.0f/music[10]);
                    L1Lpwm->write(0.5f);
                    wait((int)c[1]-48);
                    break;
            }   
         default:  
            pc.printf("error");
            break;
    }  
    // pc.printf("Playing music #%f \n\r",i);
    // L1L.period(1.0f/music[i]);
}