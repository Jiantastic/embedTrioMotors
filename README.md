# embedTrioMotors

# Testing guide

Correct source code is in <strong>./src/newCodeWithMusic.</strong>

The motor takes 3 seconds to initialise to motorHome(). Please wait for 3 seconds before you input any commands.

After 3 seconds, you should see something along the lines of:

` Rotor origin : 5 `

You can now input user commands :)

We are using a regular expression library to take in user input. User input runs in a thread, thus the user can input new commands at any given time. When the user inputs a new command, the previous command given(if there is one) is terminated and there might be a slight delay before the motor starts spinning again according to the new command. (see [Debugging](https://github.com/Jiantastic/embedTrioMotors#debugging) for more info)

User input commands are any valid regular expression according to CW2 specification.

The melody command is blocking - once the melody is started, it cannot be stopped and no other input commands can be entered. It is possible to enter other position commands such as R50V50 / V50 before entering the music command. The music will play while the motor is spinning. To input any other command, you will have to reset the board.



# Debugging

The code has been developed and tested with a power source, works fine with no stiction. Tuning parameters and motor behaviour might differ from one power source to another. 


There might be some bugs during testing.

<strong>In the event of this, please consider the following :</strong>

Pressing the reset button on the mbed board, followed by waiting for something like ` Rotor origin : 5 ` to appear before inputting a user command

If, after inputting commands the motor does not spin it sometimes might be necessary to use a finger to spin it a little. We managed to overcome this problem but conditions might be different during testing.
