# tm4c-firmware---full-duplex-communication-using-CSMA-protocol
#This repository contains the firmware for my fall 2021 semester project using TI's tm4c123gh6pm microcontroller

#main.c file contains the necessary function calls to validate and take necessary actions for the commands entered by the user into the putty terminal.
#fundef.c file contains custom definitions of all the functions used throughout the project. It also contains register configurations for PWM, ADC etc.
#fundef.h file contains the declarations of functions, global variables, structure definitions of transmit and receive messages
#uart<num>.h file contains UART0(115200 baud) and UART1(38400 baud) configurations for communicating through the terminal interface and TX and RX respectively

#Some supported commands:
# set <num1>,<num2>,<num3> -> sets the led of device assigned to channel num2 having address num1 with value num3(0-255)
# reset -> resets own device
# reset <num> -> resets device on address num
# change <num1>,<num2> -> changes device address having address num1 to num2
# sa <num1>,<num2> -> changes own device address from num1 to num2.

#note -> please refer to main file for other commands and arguments required for it to be successfully executed.
