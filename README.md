# embedTrioMotors

## To-do-list:

1. Turn the polling loop into interrupt-controlled loop 

2. Integrate the Quadrature Encoder Interface(QEI) codes into mbed and determine the formula for calculating the average or instantenous angle (position) and angular velocity.
  2.1 How to determine the time taken to complete a certain number of rotation: for evaluating the velocity
  2.2 
  
3. Set-up static (and adaptive) PD, PID controller loop in mbed
  3.1 How to determine the optimal K(s) (too large causing unstable system: root locus analysis, too small: take longer time to settle on         the desired velocity etc)
  3.2 Adaptive control implementation (Overdamped response is required)
  
4. Melody handler

5. Think of how to integrate the use of threadings,  interrupts (other than in QEI and photointerrupters), control interface etc.
  
