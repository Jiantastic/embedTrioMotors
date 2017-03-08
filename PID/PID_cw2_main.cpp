#include "PID.h"

#define RATE 0.1 //interval at which PID outputs its PWM

//Kc, Ti, Td, interval
PID controller(1.0, 0.0, 0.0, RATE);
//pv is the process value: pulse count or angle computed from the QEI output
AnalogIn pv(p15);
// co is the PID controller output in PWM, which GPIO on MCU to use to control motor and how?
PwmOut   co(p26);

int main(){

  //Analog input from 0.0 to 3.3V
  //controller.setInputLimits(0.0, 3.3);

  //Analog input from 0.0 to 360 degree or 117 pulsecount?
  controller.setInputLimits(0.0, 117.0);

  //Pwm output from 0.0 to 1.0
  //controller.setOutputLimits(0.0, 1.0);

  //Pwm output from 0.0 to Vmotor=3V
  controller.setOutputLimits(0.0, 3.0);

  //set co bias to 0 and iterate if required
  controller.setBias(0);

  //set mode to automatic tuning by PID header function (via settunings)
  controller.setMode(AUTO);

  //We want the process variable to be 1.7V
  //controller.setSetPoint(1.7);

  //setpoint value (rotation or velocity) from the regex command
  //add some processing code here to convert rotation or velocity to pulsecount or angle
  controller.setSetPoint(??); //?? to be replaced by parsed regex command


  // do we need interrupt here to replace the while loop?
  while(1){
    //Update the process variable.
    controller.setProcessValue(pv.read());
    //Set the new co controller output between outmin and outmax (0 and 3V)
    co = controller.getRealOutput(); //compute function in the header file is called to perform PID calc
    // ^might be wrong because getrealoutput cant be found in PID.cpp and PID.h so replace it with:
    //co = controller.compute(); 
    // add code here for feeding the co aka pwm to the mosfets to change the rotor state accordingly using motorout(): howwwww?
    //Wait for another loop calculation.
    wait(RATE);
  }

}
