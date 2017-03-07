# embedTrioMotors

MCU Nucleo board: https://developer.mbed.org/platforms/ST-Nucleo-F303K8/

## To-do-list:

Q [answered]: actively listening to serial port with or w/o interrupt for regex command: yes; sequential command: execute the latest cmd. 

polling or interrupt-driven serial ports:
https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/interfaces/digital/Serial/

Q [unsolved]: Do we need to use any ADC or DAC processing on the MCU (for controlling FETS using the outputs)?


Spec 1: motor will spin for a defined number of rotations and stop without overshooting. (PWM controlled torque?)

Spec 2: motor will spin at a defined angular velocity, either continuously or as a maximum while carrying out specification 1. (accessing the time interval?)

1. Turn the polling loop into interrupt-controlled loop 

2. Integrate the Quadrature Encoder Interface(QEI) codes into mbed and determine the formula for calculating the average or instantenous angle (position) and angular velocity.

  2.1 How to determine the time taken to complete a certain number of rotation: for evaluating the velocity
  
  2.2 Pulsecount or computed angle over time interval? 
  
  
3. Set-up static (and adaptive) PD, PID controller loop in mbed

  3.1 How to determine the optimal range of K(s) (too large causing unstable system: root locus analysis, too small: take longer time to         settle on the desired velocity etc), stability analysis in the report: how to achieve fast performance without overshooting and
      oscillating.
      
  3.2 PID K tuning: Adaptive control implementation (Overdamped response is required) using step test:                      https://developer.mbed.org/cookbook/PID
  
  3.3 PWM implementation: duty cycle (controllig the MOSFETS on and off time)
    
    3.3.1 Feed the controller's PWM output into one of the Nucleo's GPIO pin and to the motor PCB 
  
  3.4 Re-sinking the optical disc
  
4. Melody handler

5. Think of how to integrate the use of threadings,  interrupts (other than in QEI and photointerrupters), control interface etc.
  
