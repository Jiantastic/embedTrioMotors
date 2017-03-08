#include "mbed.h"
#include "rtos.h"
#include "implementation.h"

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

// serial config
Serial pc(SERIAL_TX, SERIAL_RX);

void inputHandler(){
    
    char input[256] = "";
    char c;
    int index = 0;
    
    do{
        c = pc.getc();
        input[index++] = c;
    }while(c != '/n');
    
    pc.printf("input is : %s",input);
    
    // IMPORTANT : I think semaphores should be used to wake up sleeping threads to do their tasks, investigate this more - THREAD SIGNAL WAIT
    // IMPORTANT : Threads should sleep after completing its task! - refer to thread state diagram
    // SLRE regex handlers, use capturing groups to get necessary data
    // if music command -> thread to musicHandler()
    // if rotation command -> thread to rotationHandler(), different types of rotation - R, V, R-V 
    // threads for inputHandler(),rotationHandler() and musicHandler(), threads should sleep if not doing work - refer to thread state diagram
    // interrupts for sampling photointerrupter and position encoder information - provisionally DONE
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

int8_t orState = motorHome();
int8_t intState = 0;
int8_t intStateOld = 0;

// sample photointerrupter output via interrupt
void readPhotoInterrupterState(){
    intState = readRotorState();
    if (intState != intStateOld) {
        intStateOld = intState;
        motorOut((intState-orState+lead+6)%6); //+6 to make sure the remainder is positive
    }
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

//-----------------------------PID-------------------------------------------

PID::PID(float Kc, float tauI, float tauD, float interval) {
 
    usingFeedForward = false;
    inAuto           = false;
 
    //Default the limits to the full range of I/O: 3.3V
    //Make sure to set these to more appropriate limits for
    //your application.
    //!!!!!!!!!!!!change this!!!!!!!!!!
    setInputLimits(0.0, 3.3);
    setOutputLimits(0.0, 3.3);
    //setInputLimits(0.0, 117.0); // 117 is the max pulsecount per rotation
    //setOutputLimits(0.0, 3.0);  //Vmotor = 3V
 
    tSample_ = interval;
 
    setTunings(Kc, tauI, tauD);
 
    setPoint_             = 0.0;
    processVariable_      = 0.0;
    prevProcessVariable_  = 0.0;
    controllerOutput_     = 0.0;
    prevControllerOutput_ = 0.0;
 
    accError_ = 0.0;
    bias_     = 0.0;
    
    realOutput_ = 0.0;
 
}
 
void PID::setInputLimits(float inMin, float inMax) {
 
    //Make sure we haven't been given impossible values.
    if (inMin >= inMax) {
        return;
    }
 
    //Rescale the working variables to reflect the changes.
    prevProcessVariable_ *= (inMax - inMin) / inSpan_;
    accError_            *= (inMax - inMin) / inSpan_;
 
    //Make sure the working variables are within the new limits.
    if (prevProcessVariable_ > 1) {
        prevProcessVariable_ = 1;
    } else if (prevProcessVariable_ < 0) {
        prevProcessVariable_ = 0;
    }
 
    inMin_  = inMin;
    inMax_  = inMax;
    inSpan_ = inMax - inMin;
 
}
 
void PID::setOutputLimits(float outMin, float outMax) {
 
    //Make sure we haven't been given impossible values.
    if (outMin >= outMax) {
        return;
    }
 
    //Rescale the working variables to reflect the changes.
    prevControllerOutput_ *= (outMax - outMin) / outSpan_;
 
    //Make sure the working variables are within the new limits.
    if (prevControllerOutput_ > 1) {
        prevControllerOutput_ = 1;
    } else if (prevControllerOutput_ < 0) {
        prevControllerOutput_ = 0;
    }
 
    outMin_  = outMin;
    outMax_  = outMax;
    outSpan_ = outMax - outMin;
 
}
 
void PID::setTunings(float Kc, float tauI, float tauD) {
 
    //Verify that the tunings make sense.
    if (Kc == 0.0 || tauI < 0.0 || tauD < 0.0) {
        return;
    }
 
    //Store raw values to hand back to user on request.
    pParam_ = Kc;
    iParam_ = tauI;
    dParam_ = tauD;
 
    float tempTauR;
 
    if (tauI == 0.0) {
        tempTauR = 0.0;
    } else {
        tempTauR = (1.0 / tauI) * tSample_;
    }
 
    //For "bumpless transfer" we need to rescale the accumulated error.
    if (inAuto) {
        if (tempTauR == 0.0) {
            accError_ = 0.0;
        } else {
            accError_ *= (Kc_ * tauR_) / (Kc * tempTauR);
        }
    }
 
    Kc_   = Kc;
    tauR_ = tempTauR;
    tauD_ = tauD / tSample_;
 
}

 
void PID::reset(void) {
 
    float scaledBias = 0.0;
 
    if (usingFeedForward) {
        scaledBias = (bias_ - outMin_) / outSpan_;
    } else {
        scaledBias = (realOutput_ - outMin_) / outSpan_;
    }
 
    prevControllerOutput_ = scaledBias;
    prevProcessVariable_  = (processVariable_ - inMin_) / inSpan_;
 
    //Clear any error in the integral.
    accError_ = 0;
 
}
 
void PID::setMode(int mode) {
 
    //We were in manual, and we just got set to auto.
    //Reset the controller internals.
    if (mode != 0 && !inAuto) {
        reset();
    }
 
    inAuto = (mode != 0);
 
}
 
void PID::setInterval(float interval) {
 
    if (interval > 0) {
        //Convert the time-based tunings to reflect this change.
        tauR_     *= (interval / tSample_);
        accError_ *= (tSample_ / interval);
        tauD_     *= (interval / tSample_);
        tSample_   = interval;
    }
 
}
 
void PID::setSetPoint(float sp) {
 
    setPoint_ = sp;
 
}
 
void PID::setProcessValue(float pv) {
 
    processVariable_ = pv;
 
}
 
void PID::setBias(float bias){
 
    bias_ = bias;
    usingFeedForward = 1;
 
}
 
float PID::compute() {
 
    //Pull in the input and setpoint, and scale them into percent span.
    float scaledPV = (processVariable_ - inMin_) / inSpan_;
 
    if (scaledPV > 1.0) {
        scaledPV = 1.0;
    } else if (scaledPV < 0.0) {
        scaledPV = 0.0;
    }
 
    float scaledSP = (setPoint_ - inMin_) / inSpan_;
    if (scaledSP > 1.0) {
        scaledSP = 1;
    } else if (scaledSP < 0.0) {
        scaledSP = 0;
    }
 
    float error = scaledSP - scaledPV;
 
    //Check and see if the output is pegged at a limit and only
    //integrate if it is not. This is to prevent reset-windup.
    if (!(prevControllerOutput_ >= 1 && error > 0) && !(prevControllerOutput_ <= 0 && error < 0)) {
        accError_ += error;
    }
 
    //Compute the current slope of the input signal.
    float dMeas = (scaledPV - prevProcessVariable_) / tSample_;
 
    float scaledBias = 0.0;
 
    if (usingFeedForward) {
        scaledBias = (bias_ - outMin_) / outSpan_;
    }
 
    //Perform the PID calculation.
    controllerOutput_ = scaledBias + Kc_ * (error + (tauR_ * accError_) - (tauD_ * dMeas));
 
    //Make sure the computed output is within output constraints.
    if (controllerOutput_ < 0.0) {
        controllerOutput_ = 0.0;
    } else if (controllerOutput_ > 1.0) {
        controllerOutput_ = 1.0;
    }
 
    //Remember this output for the windup check next time.
    prevControllerOutput_ = controllerOutput_;
    //Remember the input for the derivative calculation next time.
    prevProcessVariable_  = scaledPV;
 
    //Scale the output from percent span back out to a real world number.
    return ((controllerOutput_ * outSpan_) + outMin_);
 
}
 
float PID::getInMin() {
 
    return inMin_;
 
}
 
float PID::getInMax() {
 
    return inMax_;
 
}
 
float PID::getOutMin() {
 
    return outMin_;
 
}
 
float PID::getOutMax() {
 
    return outMax_;
 
}
 
float PID::getInterval() {
 
    return tSample_;
 
}
 
float PID::getPParam() {
 
    return pParam_;
 
}
 
float PID::getIParam() {
 
    return iParam_;
 
}
 
float PID::getDParam() {
 
    return dParam_;
 
}

