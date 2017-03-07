# embedTrioMotors

## To-do-list:

1. Turn the polling loop into interrupt-controlled loop 

2. Integrate the Quadrature Encoder Interface(QEI) codes into mbed and determine the formula for calculating the average or instantenous angle (position) and angular velocity.

  2.1 How to determine the time taken to complete a certain number of rotation: for evaluating the velocity
  
  2.2 Pulsecount or computed angle over time interval? 
  
  
3. Set-up static (and adaptive) PD, PID controller loop in mbed

  3.1 How to determine the optimal range of K(s) (too large causing unstable system: root locus analysis, too small: take longer time to         settle on the desired velocity etc), stability analysis in the report: how to achieve fast performance without overshooting and
      oscillating.
      
  3.2 PID K tuning: Adaptive control implementation (Overdamped response is required) using step test:                      https://developer.mbed.org/cookbook/PID
  
  3.3 PWM implementation: duty cycle (controllig the MOSFETS on and off time)
  
  3.4 Re-sinking the optical disc
  
4. Melody handler

5. Think of how to integrate the use of threadings,  interrupts (other than in QEI and photointerrupters), control interface etc.
  
