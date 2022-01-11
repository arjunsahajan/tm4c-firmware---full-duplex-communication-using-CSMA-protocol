/*
 * main.c
 *
 *  Created on: 04-Oct-2021
 *      Author: arjun
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "funDef.h"
#include "clock.h"
#include "uart0.h"
#include "uart1.h"
#include "wait.h"
#include "tm4c123gh6pm.h"

int main(void) {
    initHw();
    initUart0();
    initUart1();
    initSw();
    initPWM();
    initAdc0Ss3();
    setAdc0Ss3Mux(3);
    setAdc0Ss3Log2AverageCount(2);

    USER_DATA data;
    bool valid;
    uint8_t dataUI[10];
    bool ack = false;

    if(currentDeviceAddress == 0xFF) {
        setInitialAddress(1);
    }

    putsUart0("\r\nReady");
    putsUart0("\r\nYour current address is: ");
    numToString(currentDeviceAddress);

    while(true) {
        putsUart0("\r\narjun@tm4c123gxl$ ");
        getsUart0(&data);

        if(data.buffer[0] == '\0') {
            continue;
        }

    #ifdef DEBUG
        putsUart0("\r\nYou entered-> ");
        putsUart0(data.buffer);

        putsUart0("\r\n");

        parseFields(&data);

        uint8_t i = 0;
        uint8_t j = 0;

        while(j < data.fieldCount) {
            putcUart0(data.fieldType[j]);
            putsUart0("\t");
            while(i <= data.fieldPosition[j + 1]) {
                putcUart0(data.buffer[i]);
                i += 1;
            }
            putsUart0("\r\n");
            j += 1;
        }

    #endif

        valid = false;

        if(ackBit)
        {
            ack = true;
        }
        else
        {
            ack = false;
        }

        if(isCommand(&data, "set", 3))
        {
            dataUI[0] = getFieldInteger(&data, 3);
            sendRS485(getFieldInteger(&data, 1), COMMAND_SET, getFieldInteger(&data, 2), dataUI, 1, ack);
            valid   = true;
        }
        else if(isCommand(&data, "alert", 1))
        {
            valid = true;
        }
        else if(isCommand(&data, "cs", 1))
        {
            valid = true;
        }
        else if(isCommand(&data, "random", 1))
        {
            valid = true;
        }
        else if(isCommand(&data, "ack", 1))
        {
            valid = true;
        }
        else if(isCommand(&data, "sa", 2))
        {
            if(changeDeviceAddress(getFieldInteger(&data, 1), getFieldInteger(&data, 2)))
            {
                putsUart0("\r\nAddress changed. Your current address is: ");
                numToString(currentDeviceAddress);
                putsUart0("\r\n");
                valid = true;
            }
            else
            {
                putsUart0("\r\nThis is not your current address\n");
            }
        }
        else if(isCommand(&data, "poll", 0))
        {
            dataUI[0] = 0;
            sendRS485(0xFF, COMMAND_POLL, 0, dataUI, 1, ack);
            valid = true;
        }
        else if(isCommand(&data, "get", 2))
        {
            dataUI[0] = 0;
            sendRS485(getFieldInteger(&data, 1), COMMAND_GET, getFieldInteger(&data, 2), dataUI, 1, ack);
            valid = true;
        } else if(isCommand(&data, "reset", 0))
        {
            putsUart0("\r\nCommand Accepted. Resetting controller...");
            waitMicrosecond(1000000);
            initiateSystemReset();
        }
        else if(isCommand(&data, "reset", 1))
        {
            dataUI[0] = 0;
            sendRS485(getFieldInteger(&data, 1), COMMAND_RESET_NODE, 0, dataUI, 1, ack);
            valid = true;
        }
        else if(isCommand(&data, "pulse", 3))
        {
            dataUI[0] = getFieldInteger(&data, 2);
            dataUI[1] = getFieldInteger(&data, 3);
            sendRS485(getFieldInteger(&data, 1), COMMAND_PULSE, 0, dataUI, 2, ack);
            valid = true;
        }
        else if(isCommand(&data, "square", 6))
        {
            dataUI[0] = getFieldInteger(&data, 2); //level1
            dataUI[1] = getFieldInteger(&data, 3); //level2
            dataUI[2] = getFieldInteger(&data, 4); //time1
            dataUI[3] = getFieldInteger(&data, 5); //time2
            dataUI[4] = getFieldInteger(&data, 6); //cycles
            sendRS485(getFieldInteger(&data, 1), COMMAND_SQUARE, 0, dataUI, 5, ack);
            valid = true;
        }
        else if(isCommand(&data, "rgb", 4))
        {
            dataUI[0] = getFieldInteger(&data, 2);
            dataUI[1] = getFieldInteger(&data, 3);
            dataUI[2] = getFieldInteger(&data, 4);
            sendRS485(getFieldInteger(&data, 1), COMMAND_RGB, 0, dataUI, 3, ack);
            valid = true;
        }
        else if(isCommand(&data, "change", 2))
        {
            dataUI[0] = getFieldInteger(&data, 2);
            sendRS485(getFieldInteger(&data, 1), COMMAND_CHANGE_ADDRESS_NODE, 0, dataUI, 1, ack);
            valid = true;
        }

        if(!valid)
        {
            putsUart0("\r\nInvalid command");
        }
        else
        {
            putsUart0("\r\nCommand Accepted");
        }
    }
}
