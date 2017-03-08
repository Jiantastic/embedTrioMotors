#ifndef IMPLEMENTATION_H
#define	IMPLEMENTATION_H

#include "mbed.h"

//Photointerrupter input pins
#define I1pin D2
#define I2pin D11
#define I3pin D12

//Incremental encoder input pins
#define CHA   D7
#define CHB   D8  

//Motor Drive output pins   //Mask in output byte
#define L1Lpin D4           //0x01
#define L1Hpin D5           //0x02
#define L2Lpin D3           //0x04
#define L2Hpin D6           //0x08
#define L3Lpin D9           //0x10
#define L3Hpin D10          //0x20

//PID manual/auto mode 
#define MANUAL_MODE 0
#define AUTO_MODE   1

//Mapping from sequential drive states to motor phase outputs
/*
State   L1  L2  L3
0       H   -   L
1       -   H   L
2       L   H   -
3       L   -   H
4       -   L   H
5       H   L   -
6       -   -   -
7       -   -   -
*/
//Drive state to output table
const int8_t driveTable[] = {0x12,0x18,0x09,0x21,0x24,0x06,0x00,0x00};

//Mapping from interrupter inputs to sequential rotor states. 0x00 and 0x07 are not valid
const int8_t stateMap[] = {0x07,0x05,0x03,0x04,0x01,0x00,0x02,0x07};  
//const int8_t stateMap[] = {0x07,0x01,0x03,0x02,0x05,0x00,0x04,0x07}; //Alternative if phase order of input or drive is reversed

//Phase lead to make motor spin
const int8_t lead = -2;  //2 for forwards, -2 for backwards

//Status LED
extern DigitalOut led1;

//Photointerrupter inputs
extern DigitalIn I1;
extern DigitalIn I2;
extern DigitalIn I3;

// UNCOMMENT : Incremental encoder inputs
// DigitalIn CHAInput(CHA);
// DigitalIn CHBInput(CHB);

//Motor Drive outputs
extern DigitalOut L1L;
extern DigitalOut L1H;
extern DigitalOut L2L;
extern DigitalOut L2H;
extern DigitalOut L3L;
extern DigitalOut L3H;

//Position Encoder outputs
extern DigitalOut CHAInput;
extern DigitalOut CHBInput;

void inputHandler();
void rotationHandler();
void controlAlgorithm();
void musicHandler();
void motorOut(int8_t driveState);
void readPositionEncoderState();
int8_t motorHome();
void readPhotoInterrupterState();

// serial config
extern Serial pc;

//Convert photointerrupter inputs to a rotor state, 1 2 and 4 and binary powers
inline int8_t readRotorState(){
    return stateMap[I1 + 2*I2 + 4*I3];
}

// use a thread to read from serial port
inline void readFromSerial(){

}

extern int8_t orState;
extern int8_t intState;
extern int8_t intStateOld;

//--------------------------------------------PID.h---------------------------------------------------
//#define MANUAL_MODE 0
//#define AUTO_MODE   1

/**
 * Proportional-integral-derivative controller.
 */
class PID {

public:

    /**
     * Constructor.
     *
     * Sets default limits [0-3.3V], calculates tuning parameters, and sets
     * manual mode with no bias.
     *
     * @param Kc - Tuning parameter
     * @param tauI - Tuning parameter
     * @param tauD - Tuning parameter
     * @param interval PID calculation performed every interval seconds.
     */
    PID(float Kc, float tauI, float tauD, float interval);

    /**
     * Scale from inputs to 0-100%.
     *
     * @param InMin The real world value corresponding to 0%.
     * @param InMax The real world value corresponding to 100%.
     */
    void setInputLimits(float inMin , float inMax);

    /**
     * Scale from outputs to 0-100%.
     *
     * @param outMin The real world value corresponding to 0%.
     * @param outMax The real world value corresponding to 100%.
     */
    void setOutputLimits(float outMin, float outMax);

    /**
     * Calculate PID constants.
     *
     * Allows parameters to be changed on the fly without ruining calculations.
     *
     * @param Kc - Tuning parameter
     * @param tauI - Tuning parameter
     * @param tauD - Tuning parameter
     */
    void setTunings(float Kc, float tauI, float tauD);

    /**
     * Reinitializes controller internals. Automatically
     * called on a manual to auto transition.
     */
    void reset(void);

    /**
     * Set PID to manual or auto mode.
     *
     * @param mode        0 -> Manual
     *             Non-zero -> Auto
     */
    void setMode(int mode);

    /**
     * Set how fast the PID loop is run.
     *
     * @param interval PID calculation peformed every interval seconds.
     */
    void setInterval(float interval);

    /**
     * Set the set point.
     *
     * @param sp The set point as a real world value.
     */
    void setSetPoint(float sp);

    /**
     * Set the process value.
     *
     * @param pv The process value as a real world value.
     */
    void setProcessValue(float pv);

    /**
     * Set the bias.
     *
     * @param bias The bias for the controller output.
     */
    void setBias(float bias);

    /**
     * PID calculation.
     *
     * @return The controller output as a float between outMin and outMax.
     */
    float compute(void);

    //Getters.
    float getInMin();
    float getInMax();
    float getOutMin();
    float getOutMax();
    float getInterval();
    float getPParam();
    float getIParam();
    float getDParam();

private:
  //add 'extern' to the variable declaration in .cpp??
    bool usingFeedForward;
    bool inAuto;

    //Actual tuning parameters used in PID calculation.
    float Kc_;
    float tauR_;
    float tauD_;

    //Raw tuning parameters.
    float pParam_;
    float iParam_;
    float dParam_;

    //The point we want to reach.
    float setPoint_;
    //The thing we measure.
    float processVariable_;
    float prevProcessVariable_;
    //The output that affects the process variable.
    float controllerOutput_;
    float prevControllerOutput_;

    //We work in % for calculations so these will scale from
    //real world values to 0-100% and back again.
    float inMin_;
    float inMax_;
    float inSpan_;
    float outMin_;
    float outMax_;
    float outSpan_;

    //The accumulated error, i.e. integral.
    float accError_;
    //The controller output bias.
    float bias_;

    //The interval between samples.
    float tSample_;

    //Controller output as a real world value.
    volatile float realOutput_;

};

#endif
