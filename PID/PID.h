/**
 * @author Brett Beauregard
 *
 * @section LICENSE
 *
 * Copyright (c) 2010 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 * 
 * A PID controller is a widely used feedback controller commonly found in
 * industry.
 *
 * This library is a port of Brett Beauregard's Arduino PID library:
 *
 *  https://github.com/br3ttb/Arduino-PID-Library
 *
 * The wikipedia article on PID controllers is a good place to start on
 * understanding how they work:
 *
 *  http://en.wikipedia.org/wiki/PID_controller
 *
 * For a clear and elegant explanation of how to implement and tune a
 * controller, the controlguru website by Douglas J. Cooper (who also happened
 * to be Brett's controls professor) is an excellent reference:
 *
 *  http://www.controlguru.com/
 */
 
 
/**
 * Includes
 */
#include "mbed.h"
 
/**
 * Defines
 */
 
#ifndef PID_H
#define PID_H

#define LIBRARY_VERSION 1.1.1

class PID
{


  public:

  //Constants used in some of the functions below
  #define AUTOMATIC 1
  #define MANUAL    0
  #define DIRECT  0
  #define REVERSE  1

  //commonly used functions **************************************************************************
    PID(float*, float*, float*,        // * constructor.  links the PID to the Input, Output, and 
        float, float, float, int);     //   Setpoint.  Initial tuning parameters are also set here
    
    void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute();                       // * performs the PID calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(float, float); //clamps the output to a specific range. 0-1.0 by default, but
                                          //it's likely the user will want to change this depending on
                                          //the application
    


  //available but not commonly used functions ********************************************************
    void SetTunings(float, float,       // * While most users will set the tunings once in the 
                    float);              //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
    void SetControllerDirection(int);     // * Sets the Direction, or "Action" of the controller. DIRECT
                                          //   means the output will increase when error is positive. REVERSE
                                          //   means the opposite.  it's very unlikely that this will be needed
                                          //   once it is set in the constructor.
    void SetSampleTime(int);              // * sets the frequency, in Milliseconds, with which 
                                          //   the PID calculation is performed.  default is 100
                                          
                                          
                                          
  //Display functions ****************************************************************
    float GetKp();                       // These functions query the pid for interal values.
    float GetKi();                       //  they were created mainly for the pid front-end,
    float GetKd();                       // where it's important to know what is actually 
    int GetMode();                        //  inside the PID.
    int GetDirection();                   //

  private:
    void Initialize();
    
    float dispKp;              // * we'll hold on to the tuning parameters in user-entered 
    float dispKi;              //   format for display purposes
    float dispKd;              //
    
    float kp;                  // * (P)roportional Tuning Parameter
    float ki;                  // * (I)ntegral Tuning Parameter
    float kd;                  // * (D)erivative Tuning Parameter

    int controllerDirection;

    float *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    float *myOutput;             //   This creates a hard link between the variables and the 
    float *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
              
    unsigned long lastTime;
    float ITerm, lastInput;

    unsigned long SampleTime;
    float outMin, outMax;
    bool inAuto;
};
#endif

