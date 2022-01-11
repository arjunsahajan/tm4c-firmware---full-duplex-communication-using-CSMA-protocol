/*
 * funDef.c
 *
 *  Created on: 02-Nov-2021
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

//************************* HARDWARE INITIALISATION ********************************

void initHw(void)
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Peripheral configuration
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4 | SYSCTL_RCGCGPIO_R5;
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    _delay_cycles(3);

    // Hardware LED configurations
    GPIO_PORTE_DIR_R  |= GREEN_LED_MASK | RED_LED_MASK;
    GPIO_PORTE_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK;
    GPIO_PORTE_DEN_R  |= GREEN_LED_MASK | RED_LED_MASK;

    // ADC pin configuration
    GPIO_PORTE_AFSEL_R |= AIN3_MASK;
    GPIO_PORTE_DEN_R &= ~AIN3_MASK;
    GPIO_PORTE_AMSEL_R |= AIN3_MASK;

    // Push button 1 configuration
    GPIO_PORTF_DIR_R &= ~(PB1_MASK);
    GPIO_PORTF_DR2R_R  |= PB1_MASK;
    GPIO_PORTF_DEN_R   |= PB1_MASK;
    GPIO_PORTF_PUR_R   |= PB1_MASK;

    // Timer configuration
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
    TIMER1_TAILR_R = 400000;
    TIMER1_IMR_R = TIMER_IMR_TATOIM;
    TIMER1_CTL_R |= TIMER_CTL_TAEN;
    NVIC_EN0_R |= 1 << (INT_TIMER1A - 16);
}

//************************* HARDWARE INITIALISATION ********************************

//************************* PWM MODULE INITIALISATION ********************************

void initPWM(void)
{
    // PORTF alternate function configuration for PWM
    GPIO_PORTF_DIR_R   |= RED_LED_ONBOARD_MASK | BLUE_LED_ONBOARD_MASK | GREEN_LED_ONBOARD_MASK;
    GPIO_PORTF_DR2R_R  |= RED_LED_ONBOARD_MASK | BLUE_LED_ONBOARD_MASK | GREEN_LED_ONBOARD_MASK;
    GPIO_PORTF_DEN_R   |= RED_LED_ONBOARD_MASK | BLUE_LED_ONBOARD_MASK | GREEN_LED_ONBOARD_MASK;
    GPIO_PORTF_AFSEL_R |= RED_LED_ONBOARD_MASK | BLUE_LED_ONBOARD_MASK | GREEN_LED_ONBOARD_MASK;
    GPIO_PORTF_PCTL_R  &= ~(GPIO_PCTL_PF1_M | GPIO_PCTL_PF2_M | GPIO_PCTL_PF3_M);
    GPIO_PORTF_PCTL_R  |= GPIO_PCTL_PF1_M1PWM5 | GPIO_PCTL_PF2_M1PWM6 | GPIO_PCTL_PF3_M1PWM7;

    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;
    SYSCTL_SRPWM_R = 0;
    PWM1_2_CTL_R = 0;
    PWM1_3_CTL_R = 0;

    // Generator setup
    PWM1_2_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
    PWM1_3_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
    PWM1_3_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;

    // PWM load
    PWM1_2_LOAD_R = 256;
    PWM1_3_LOAD_R = 256;

    // Compare value init
    PWM1_2_CMPB_R = 0;
    PWM1_3_CMPB_R = 0;
    PWM1_3_CMPA_R = 0;

    PWM1_2_CTL_R = PWM_0_CTL_ENABLE;
    PWM1_3_CTL_R = PWM_0_CTL_ENABLE;
    PWM1_ENABLE_R = PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN;
}

//************************* PWM MODULE INITIALISATION ********************************

//************************* INIT ALL GLOABAL VARIABLES********************************

void initSw(void)
{
    uint8_t i            = 0;
    msgInProgress        = -1;
    msgTXphase           = 0;
    msgRXphase           = 0;
    TX_ENABLE            = 0;
    currentDeviceAddress = FLASH_ADDRESS[0];
    GREEN_LED            = 0;
    RED_LED              = 0;
    readIndexMsgFIFO     = 0;
    writeIndexMsgFIFO    = 0;
    rxBufferReadIndex    = 0;
    rxBufferWriteIndex   = 0;
    raw                  = 0;
    txLEDtimeout         = 0;
    rxLEDtimeout         = 0;
    ackBit               = false;
    randomBit            = false;
    csBit                = false;
    busyBit              = false;
    testCS               = 0;

    // init all TX messages
    while(i < TX_MSG_ARRAY_SIZE)
    {
        TX485MsgsArray[i].valid = false;
        i += 1;
    }

    // init wave form generation structure
    wave.valid  = false;
    wave.phase  = 0;
    wave.deltaT = 0;
    wave.count  = 0;
    wave.time1  = 0;
    wave.time2  = 0;
    wave.level1 = 0;
    wave.level2 = 0;

}

//************************* INIT ALL GLOABAL VARIABLES********************************


//************************* ADC FUNCTIONS ********************************************

void initAdc0Ss3(void)
{
    // Enable clocks
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
    _delay_cycles(16);

    // Configure ADC
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
    ADC0_CC_R = ADC_CC_CS_SYSPLL;
    ADC0_PC_R = ADC_PC_SR_1M;
    ADC0_EMUX_R = ADC_EMUX_EM3_PROCESSOR;
    ADC0_SSCTL3_R = ADC_SSCTL3_END0;
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;
}

void setAdc0Ss3Log2AverageCount(uint8_t log2AverageCount)
{
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
    ADC0_SAC_R = log2AverageCount;
    if (log2AverageCount == 0)
        ADC0_CTL_R &= ~ADC_CTL_DITHER;
    else
        ADC0_CTL_R |= ADC_CTL_DITHER;
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;
}

void setAdc0Ss3Mux(uint8_t input)
{
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
    ADC0_SSMUX3_R = input;
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;
}

int16_t readAdc0Ss3(void)
{
    ADC0_PSSI_R |= ADC_PSSI_SS3;
    while (ADC0_ACTSS_R & ADC_ACTSS_BUSY);
    while (ADC0_SSFSTAT3_R & ADC_SSFSTAT3_EMPTY);
    return ADC0_SSFIFO3_R;
}

//************************* ADC FUNCTIONS ********************************


//************************* GET USER DATA ********************************

void getsUart0(USER_DATA *dataPtr)
{
    uint8_t count = 0;
    char c;

    while(true)
        {
            dataPtr -> buffer[count] = getcUart0();
            c = dataPtr -> buffer[count];

            if((c == 8 || c == 127) && count == 0)
            {
                continue;
            }

            putcUart0(c);

            if(c >= 32 && c <= 122)
            {
                count += 1;
            } else if((c == 8 || c == 127) && count > 0)
            {
                count -= 1;
            } else if(c == 13 || c == 10)
            {
                dataPtr -> buffer[count] = '\0';
                break;
            }

            if(count == MAX_CHARS)
            {
                dataPtr -> buffer[count] = '\0';
                break;
            }
        }
}

//************************* GET USER DATA ********************************


//************************* PROCESS USER INPUT ********************************

void parseFields(USER_DATA* dataPtr)
{
    uint8_t i = 1;
    uint8_t j = 1;

    dataPtr -> fieldCount       = 1;
    dataPtr -> fieldType[0]     = 'a';
    dataPtr -> fieldPosition[0] = 0;

    while(true)
    {
        if(dataPtr -> buffer[i] == '\0' || dataPtr -> fieldCount == MAX_FIELDS - 1)
        {
            dataPtr ->  fieldPosition[j] = i;
            dataPtr ->  fieldType[j] = '\0';
            return;
        }

        if((dataPtr -> buffer[i] < 'A' || dataPtr -> buffer[i] > 'Z') && (dataPtr -> buffer[i] < 'a' || dataPtr -> buffer[i] > 'z') && (dataPtr -> buffer[i] < '0' || dataPtr -> buffer[i] > '9'))
        {
            dataPtr -> buffer[i]         = ' ';
            dataPtr ->  fieldPosition[j] = i;
            if(dataPtr -> buffer[i + 1] >= 'A' && dataPtr -> buffer[i + 1] <= 'z')
            {
                dataPtr -> fieldType[j] = 'a';
                dataPtr -> fieldCount  += 1;
                j += 1;
            }
            else if(dataPtr -> buffer[i + 1] >= '0' && dataPtr -> buffer[i + 1] <= '9')
            {
                dataPtr -> fieldType[j] = 'n';
                dataPtr -> fieldCount  += 1;
                j += 1;
            }
        }
        i += 1;
    }
}

//************************* PROCESS USER INPUT ********************************


//************************* STRING FUNCTIONS ********************************

uint8_t strLength(const char* word)
{
    uint8_t strLen = 0;

    while(word[strLen] != ' ' && word[strLen] != '\0')
    {
        strLen += 1;
    }
    return strLen;
}

bool strCompare(const char* inputParam, const char* word)
{
    uint8_t i                = 0;
    uint8_t inputParamLength = strLength(inputParam);
    uint8_t wordLength       = strLength(word);

    if(inputParamLength != wordLength)
    {
        return 0;
    }

    while(i < wordLength)
    {
        if(word[i] != inputParam[i])
        {
            return 0;
        }
        i += 1;
    }
    return 1;
}

uint8_t stringToNum(const char* numString)
{
    uint8_t sizeOfString = 0;
    uint8_t singleNum    = 0;
    uint8_t multiplier   = 1;
    uint8_t finalNum     = 0;

    sizeOfString = strLength(numString);

    while(sizeOfString > 0)
    {
        singleNum     = numString[sizeOfString - 1] - '0';
        finalNum     += multiplier * singleNum;
        multiplier   *= 10;
        sizeOfString -= 1;
    }

    return finalNum;
}

char* reverseNum(uint8_t num)
{
    uint8_t i = 0;
    uint8_t remainder;
    static char arr[10];

    while (num != 0)
    {
        remainder = num % 10;
        arr[i] = remainder + '0';
        num /= 10;
        i += 1;
    }
    arr[i] = '\0';

    return arr;
}

void numToString(uint8_t num)
{
    char* revArr = reverseNum(num);
    uint8_t size = strLength(revArr);

    while(size > 0)
    {
        putcUart0(revArr[size - 1]);
        size -= 1;
    }
}

uint32_t getFieldInteger(USER_DATA* dataPtr, uint8_t fieldNumber)
{
    if(fieldNumber <= dataPtr -> fieldCount)
    {
            return stringToNum(&(dataPtr -> buffer[dataPtr -> fieldPosition[fieldNumber] + 1]));
    }
    return 0;
}

char* getFieldString(USER_DATA* dataPtr, uint8_t fieldNumber)
{
    if(fieldNumber <= dataPtr -> fieldCount) {
        return &(dataPtr -> buffer[dataPtr -> fieldPosition[fieldNumber] + 1]);
    }
    return NULL;
}

//************************* STRING FUNCTIONS ********************************


//************************* USER INPUT VALIDATION ********************************

uint8_t countArguments(USER_DATA* dataPtr, uint8_t offset)
{
    uint8_t count = 0;

    while(dataPtr -> fieldType[offset] != '\0')
    {
        if(dataPtr -> fieldType[offset] == 'n' || dataPtr -> fieldType[offset] == 'a') {
            count += 1;
        }
        offset += 1;
    }
    return count;
}

bool isCommand(USER_DATA* dataPtr, const char* command, uint8_t minArgs) {
    if(countArguments(dataPtr, 1) != minArgs) {
        return 0;
    }

    if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "alert") && strCompare(command, "alert"))
    {
        if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "off") || strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "on"))
        {
            return 1;
        }
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "set") && strCompare(command, "set"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "reset") && strCompare(command, "reset"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "random") && strCompare(command, "random"))
    {
        if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "off"))
        {
            randomBit = true;
            return 1;
        }
        else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "on"))
        {
            randomBit = false;
            return 1;
        }
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "ack") && strCompare(command, "ack"))
    {
        if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "off"))
        {
            ackBit = false;
            return 1;
        }
        else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "on")) {
            ackBit = true;
            return 1;
        }
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "sa") && strCompare(command, "sa"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "poll") && strCompare(command, "poll"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "get") && strCompare(command, "get"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "pulse") && strCompare(command, "pulse"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "square") && strCompare(command, "square"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "rgb") && strCompare(command, "rgb"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "change") && strCompare(command, "change"))
    {
        return 1;
    }
    else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[0]]), "cs") && strCompare(command, "cs"))
    {
        if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "off"))
        {
            csBit = false;
            return 1;
        }
        else if(strCompare(&(dataPtr -> buffer[dataPtr -> fieldPosition[1] + 1]), "on")) {
            csBit = true;
            return 1;
        }
    }

    return 0;
}

//************************* USER INPUT VALIDATION ********************************

uint8_t calculatePower(uint8_t num, uint8_t exponent)
{
    uint8_t i = 0;
    while(i < exponent)
    {
        num *= num;
        i += 1;
    }

    return num;
}

uint8_t calculateChecksum(uint16_t dstAddress, uint8_t srcAddress, uint8_t command, uint8_t sequenceID, uint8_t channel, uint8_t size, uint8_t data[])
{
    uint8_t checksum = dstAddress + srcAddress + command + sequenceID + channel + size;
    uint8_t i = 0;
    while(i < size)
    {
        checksum += data[i];
        i += 1;
    }

    return ~checksum;
}

//************************* PACKET GENERATION AND TRANSMISSION ********************************

void sendRS485(uint8_t dstAddress, uint8_t command, uint8_t channelNo, uint8_t dataUI[], uint8_t dataSize, bool ack)
{
    static uint8_t sequenceID = 0;
    uint8_t i = 0;
    bool full;
    char queueMsg[50];

    full = ((writeIndexMsgFIFO + 1) & (TX_MSG_ARRAY_SIZE - 1)) == readIndexMsgFIFO;
    if (!TX485MsgsArray[writeIndexMsgFIFO].valid && !full)
    {
        TX485MsgsArray[writeIndexMsgFIFO].dstAddress     = dstAddress;
        TX485MsgsArray[writeIndexMsgFIFO].srcAddress     = currentDeviceAddress;
        TX485MsgsArray[writeIndexMsgFIFO].command        = command | (ack << 7);
        TX485MsgsArray[writeIndexMsgFIFO].channel        = channelNo;
        TX485MsgsArray[writeIndexMsgFIFO].size           = dataSize;
        TX485MsgsArray[writeIndexMsgFIFO].countTX        = 0;
        TX485MsgsArray[writeIndexMsgFIFO].timeToTransmit = 0;
        TX485MsgsArray[writeIndexMsgFIFO].sequenceID     = sequenceID % 256;

        while(i < dataSize)
        {
            TX485MsgsArray[writeIndexMsgFIFO].data[i]    = dataUI[i];
            i += 1;
        }

        TX485MsgsArray[writeIndexMsgFIFO].checksum       = calculateChecksum(TX485MsgsArray[writeIndexMsgFIFO].dstAddress, TX485MsgsArray[writeIndexMsgFIFO].srcAddress, TX485MsgsArray[writeIndexMsgFIFO].command, TX485MsgsArray[writeIndexMsgFIFO].sequenceID, TX485MsgsArray[writeIndexMsgFIFO].channel, TX485MsgsArray[writeIndexMsgFIFO].size, TX485MsgsArray[writeIndexMsgFIFO].data);
        TX485MsgsArray[writeIndexMsgFIFO].valid = true;
    }

    writeIndexMsgFIFO = (writeIndexMsgFIFO + 1) & (TX_MSG_ARRAY_SIZE - 1);

    sprintf(queueMsg, "\n\rSequence ID %d queued", sequenceID);
    userInterfaceTx(queueMsg);

    sequenceID += 1;

    if(UART1_FR_R & UART_FR_TXFE)                        // Manually prime the pump
    {
        sendRS485Byte();
    }
}

void sendRS485Byte(void)
{
    static uint8_t j = 0;
    static bool found = false;
    uint8_t i = 0;
    char msgUI[30];

    if(msgInProgress == -1)
    {
        while(true)
        {
            if(TX485MsgsArray[i].valid && TX485MsgsArray[i].timeToTransmit == 0)
            {
                msgInProgress = i;
                found = true;
                TX_ENABLE = 1;
                break;
            }

            i += 1;
        }
    }

    if(msgTXphase == 0 && TX485MsgsArray[msgInProgress].valid)
    {
        if(csBit)
        {
            if(testDone)
            {
                testDone = false;
                if(!busyBit)
                  {
                      itsTimeToSend = true;
                  }
                  else
                  {
                      itsTimeToSend = false;
                  }
              }
              else
              {
                  busyBit = false;
                  testDone = false;
                  testCS = 2;
              }
          }
          else
          {
              itsTimeToSend = true;
          }
      }

    if(msgTXphase == 0)
    {
        UART1_LCRH_R &= ~(UART_LCRH_EPS);
    }
    else if(msgTXphase == 1)
    {
        UART1_LCRH_R |= UART_LCRH_EPS;
    }

    if(found && itsTimeToSend)
    {
        if(msgTXphase == 0)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].dstAddress;
            msgTXphase += 1;
        }
        else if(msgTXphase == 1)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].srcAddress;
            msgTXphase += 1;
        }
        else if(msgTXphase == 2)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].sequenceID;
            msgTXphase += 1;
        }
        else if(msgTXphase == 3)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].command;
            msgTXphase += 1;
        }
        else if(msgTXphase == 4)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].channel;
            msgTXphase += 1;
        }
        else if(msgTXphase == 5)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].size;
            msgTXphase += 1;
        }
        else if(msgTXphase == 6)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].data[j];
            j += 1;
            if(j == TX485MsgsArray[msgInProgress].size) {
                msgTXphase += 1;
            }
        }
        else if(msgTXphase == 7)
        {
            UART1_DR_R = TX485MsgsArray[msgInProgress].checksum;
            msgTXphase += 1;
            found = false;
        }
    }
    else if(UART1_CTL_R & UART_CTL_EOT)
    {
        TX_ENABLE = 0;
        RED_LED = 1;
        txLEDtimeout = TX_LED_TIMEOUT;

        if(!(TX485MsgsArray[msgInProgress].command & 0x80))
        {
            TX485MsgsArray[msgInProgress].valid = false;
            msgInProgress = -1;
        }
        else if(TX485MsgsArray[msgInProgress].command & 0x80)
        {
            found = true;
            TX485MsgsArray[msgInProgress].countTX += 1;
            if(TX485MsgsArray[msgInProgress].countTX == MAX_RE_TX_COUNT)
            {
                RED_LED = 1;
                txLEDtimeout = 0;
                TX485MsgsArray[msgInProgress].valid = false;
                sprintf(msgUI, "\n\rError sending message %d\n\r", TX485MsgsArray[msgInProgress].sequenceID);
                userInterfaceTx(msgUI);
                msgInProgress = -1;
                found = false;
            }
            else
            {
                if(!randomBit)
                {
                    TX485MsgsArray[msgInProgress].timeToTransmit =  MIN_BACK_OFF_TIME + calculatePower(2, TX485MsgsArray[msgInProgress].countTX);
                    TX485MsgsArray[msgInProgress].timeToTransmit *= RE_TX_MULTIPLIER;
                } else
                {
                    seed = ((readAdc0Ss3() & 0xFF) ^ 0xAAA) + currentDeviceAddress;
                    TX485MsgsArray[msgInProgress].timeToTransmit = MIN_BACK_OFF_TIME + seed * (calculatePower(2, TX485MsgsArray[msgInProgress].countTX));
                    TX485MsgsArray[msgInProgress].timeToTransmit *= RE_TX_MULTIPLIER;
                }
            }
        }
        msgTXphase = 0;
        j = 0;
    }
}

//************************* PACKET GENERATION AND TRANSMISSION ********************************

//************************* TIMER ISR FOR RETRANSMISSION, LED TIMEOUT CARRIER SENSE AND PATTERN GENERATION ********************************

void timer1Isr()
{
    char msgUI[30];

    if(TX485MsgsArray[msgInProgress].valid)
    {
        if(TX485MsgsArray[msgInProgress].timeToTransmit > 0)
        {
            TX485MsgsArray[msgInProgress].timeToTransmit -= 1;
            if(TX485MsgsArray[msgInProgress].timeToTransmit == 0)
            {
                sprintf(msgUI, "\n\rRetransmitting -> Attempt %d\n", TX485MsgsArray[msgInProgress].countTX);
                userInterfaceTx(msgUI);
                if(!(UART1_RIS_R & UART_RIS_TXRIS))
                {
                    if(UART1_FR_R & UART_FR_TXFE)
                    {
                        sendRS485Byte();
                    }
                }
            }
        }
    }

    if(txLEDtimeout > 0)
    {
        txLEDtimeout -= 1;
        if(txLEDtimeout == 0)
        {
            RED_LED = 0;
        }
    }

    if(rxLEDtimeout > 0)
    {
        rxLEDtimeout -= 1;
        if(rxLEDtimeout == 0)
        {
            GREEN_LED = 0;
        }
    }

    if(testCS > 0)
    {
        testCS -= 1;
        if(testCS == 0)
        {
            testDone = true;
        }
    }

    if(wave.valid == true)
    {
        wave.deltaT += 1;
        if(wave.phase == 0)
        {
            if(wave.deltaT == wave.time1)
            {
                wave.phase = 1;
                if(pulseBit)
                {
                    PWM1_2_CMPB_R = wave.level1;
                    wave.valid = false;
                    wave.deltaT = 0;
                    pulseBit = false;
                }
                else if(squareBit)
                {
                    PWM1_2_CMPB_R = wave.level1;
                    wave.deltaT = 0;
                }
            }
        }
        else if(wave.phase == 1)
        {
            if(wave.deltaT == wave.time2)
            {
                PWM1_2_CMPB_R = wave.level2;
                wave.deltaT = 0;
                wave.phase = 0;
                wave.count -= 1;
                if(wave.count == 0)
                {
                    squareBit = false;
                    wave.valid = false;
                    PWM1_2_CMPB_R = 0;
                }
            }
        }
    }

    TIMER1_ICR_R = TIMER_ICR_TATOCINT;

}

//************************* TIMER ISR FOR RETRANSMISSION, LED TIMEOUT CARRIER SENSE AND PATTERN GENERATION ********************************

//************************* UART1 ISR FOR TRANSMISSON AND RECEPTION ********************************

void uart1Isr(void) {

    if(UART1_RIS_R & UART_RIS_TXRIS) {
        if(UART1_FR_R & UART_FR_TXFE)
        {
            sendRS485Byte();
        }
        UART1_ICR_R = UART_ICR_TXIC;
    }

    if(UART1_RIS_R & UART_RIS_RXRIS) {
        if(UART1_FR_R & UART_FR_RXFF)
        {
            static uint8_t i = 0;
            if(msgRXphase == 0)
            {
                if((UART1_DR_R & UART_DR_PE) && (((UART1_DR_R & 0xFF) == 0xFF) || ((UART1_DR_R & 0xFF) == currentDeviceAddress)))
                {
                    msgRXphase += 1;
                    busyBit = true;
                    rxMsg.dstAddress = UART1_DR_R & 0xFF;

                }
            }
            else if(msgRXphase == 1)
            {
                msgRXphase += 1;
                rxMsg.srcAddress = UART1_DR_R & 0xFF;
            }
            else if(msgRXphase == 2)
            {
                msgRXphase += 1;
                rxMsg.sequenceID = UART1_DR_R & 0xFF;
            }
            else if(msgRXphase == 3)
            {
                msgRXphase += 1;
                rxMsg.command = UART1_DR_R & 0xFF;
            }
            else if(msgRXphase == 4)
            {
                msgRXphase += 1;
                rxMsg.channel = UART1_DR_R & 0xFF;
            }
            else if(msgRXphase == 5)
            {
                msgRXphase += 1;
                rxMsg.size = UART1_DR_R & 0xFF;
            }
            else if(msgRXphase == 6)
            {
                if(i == rxMsg.size)
                {
                    msgRXphase += 1;
                }
                else
                {
                    rxMsg.data[i] = UART1_DR_R & 0xFF;
                    i += 1;
                }
            }
            else if(msgRXphase == 7)
            {
                msgRXphase += 1;
                rxMsg.checksum = UART1_DR_R & 0xFF;
                if(rxMsg.checksum == calculateChecksum(rxMsg.dstAddress, rxMsg.srcAddress, rxMsg.command, rxMsg.sequenceID, rxMsg.channel, rxMsg.size, rxMsg.data))
                {
                    GREEN_LED = 1;
                    rxLEDtimeout = RX_LED_TIMEOUT;
                    processData(&rxMsg);
                }
                else
                {
                    GREEN_LED = 1;
                }
                i = 0;
                msgRXphase = 0;
                UART1_ICR_R = UART_ICR_RXIC;
            }
        }
    }
}

//************************* UART1 ISR FOR TRANSMISSON AND RECEPTION ********************************

//************************* PROCESS RECEIVED DATA ********************************

void processData(RX485Msg* rxMsgPtr)
{
    char msgUI[100];
    uint8_t data[1];

// ********************** ACTIONS TO BE PERFORMED IF A COMMAND IS RECEIVED ************************************

    if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_SET)                   // if a set command is received
    {
        if(rxMsgPtr -> channel == 1)
        {
            PWM1_2_CMPB_R = rxMsgPtr -> data[0];
        }
        else if(rxMsgPtr -> channel == 2)
        {
            PWM1_3_CMPA_R = rxMsgPtr -> data[0];
        }
        else if(rxMsgPtr -> channel == 3)
        {
            PWM1_3_CMPB_R = rxMsgPtr -> data[0];
        }
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_GET)             // if a data request is received
    {
        if(rxMsgPtr -> channel == 1)
        {
            rxMsgPtr -> data[0] = PWM1_2_CMPB_R;
        }
        else if(rxMsgPtr -> channel == 2)
        {
            rxMsgPtr -> data[0] = PWM1_3_CMPA_R;
        }
        else if(rxMsgPtr -> channel == 3)
        {
            rxMsgPtr -> data[0] = PWM1_3_CMPB_R;
        }
        else if(rxMsgPtr -> channel == 4)
        {
            rxMsgPtr -> data[0] = PB1;
        }
        sendRS485(rxMsgPtr -> srcAddress, COMMAND_DATA_REPORT, rxMsgPtr -> channel, rxMsgPtr -> data, 1, 0);
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK)== COMMAND_DATA_REPORT)                          // if a data report is received
    {
        sprintf(msgUI, "\n\rChannel %d was %d\n", rxMsgPtr -> channel, rxMsgPtr -> data[0]);
        userInterfaceTx(msgUI);
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK)== COMMAND_RGB)                                   // if an rgb command is received
    {
        PWM1_2_CMPB_R = rxMsgPtr -> data[0];
        PWM1_3_CMPA_R = rxMsgPtr -> data[1];
        PWM1_3_CMPB_R = rxMsgPtr -> data[2];
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_RESET_NODE)                          // if a reset command is received
    {
        putsUart0("\r\nCommand Accepted. Resetting controller...");
        waitMicrosecond(2000000);
        initiateSystemReset();
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_POLL)                                 // if poll command is received
    {
        sprintf(msgUI, "\n\rPoll request received from address %d\n", rxMsgPtr -> srcAddress);
        userInterfaceTx(msgUI);
        data[0] = rxMsgPtr -> sequenceID;
        sendRS485(rxMsgPtr -> srcAddress, COMMAND_POLL_RESPONSE, 0, data, 1, 0);
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_POLL_RESPONSE)                        // if response to poll is received
    {
        sprintf(msgUI, "\n\rPoll response received from address %d\n", rxMsgPtr -> srcAddress);
        userInterfaceTx(msgUI);
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_CHANGE_ADDRESS_NODE)                  // if a change address command is received
    {
        if(changeDeviceAddress(rxMsgPtr -> dstAddress, rxMsgPtr -> data[0]))
        {
            sprintf(msgUI, "\n\rAddress changed. Your current address is: %d\n", currentDeviceAddress);
            userInterfaceTx(msgUI);
        }
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_PULSE)
    {
        pulseBit      = true;
        wave.time1    = rxMsgPtr -> data[0];
        wave.level1   = rxMsgPtr -> data[1];
        wave.phase    = 0;
        PWM1_2_CMPB_R = 255;
        wave.valid    = true;
    }
    else if((rxMsgPtr -> command & COMMAND_ACK_CHECK) == COMMAND_SQUARE)
    {
        squareBit   = true;
        wave.level1 = rxMsgPtr -> data[0];
        wave.level2 = rxMsgPtr -> data[1];
        wave.time1  = rxMsgPtr -> data[2];
        wave.time2  = rxMsgPtr -> data[3];
        wave.count  = rxMsgPtr -> data[4];
        wave.phase  = 0;
        wave.valid  = true;
    }

    // ********************** ACTIONS TO BE PERFORMED IF A COMMAND IS RECEIVED ************************************

    if(rxMsgPtr -> command & 0x80)                            // if an ack is requested
       {
           data[0] = rxMsgPtr -> sequenceID;
           sendRS485(rxMsgPtr -> srcAddress, COMMAND_ACK_RESPOSNE, rxMsgPtr -> channel, data, 1, 0);
       }
       else if(rxMsgPtr -> command == COMMAND_ACK_RESPOSNE)   // if ack response is received
       {
           uint8_t i = 0;
           while(true) {
               if(TX485MsgsArray[i].valid)
               {
                   if(TX485MsgsArray[i].sequenceID == rxMsgPtr -> data[0] && (TX485MsgsArray[i].dstAddress == rxMsgPtr -> srcAddress || TX485MsgsArray[i].dstAddress == 0xFF))
                    {
                        TX485MsgsArray[i].valid = false;
                        msgInProgress = -1;
                        sprintf(msgUI, "\n\rMessage %d ack received\n", rxMsgPtr -> data[0]);
                        userInterfaceTx(msgUI);
                        break;
                    }
               }

               i += 1;

               if(i == TX_MSG_ARRAY_SIZE)
               {
                   break;
               }
           }
       }
}

//************************* PROCESS RECEIVED DATA ********************************

//************************* NOTIFY USER AFTER PROCESSING DATA ********************************

void userInterfaceTx(char* msg) {
    bool full;
    uint8_t i = 0;

    full = ((rxBufferWriteIndex + 1) & (RX_BUFFER_SIZE - 1)) == rxBufferReadIndex;
    if(!full)
    {
        while(msg[i] != '\0')
        {
            rxBuffer[rxBufferWriteIndex] = msg[i];
            rxBufferWriteIndex = (rxBufferWriteIndex + 1) & (RX_BUFFER_SIZE - 1);
            i += 1;
        }
    }

    if(UART0_FR_R & UART_FR_TXFE)
    {
        sendUIByte();
    }
}

void sendUIByte(void)
{
    UART0_DR_R = rxBuffer[rxBufferReadIndex];
    rxBufferReadIndex = (rxBufferReadIndex + 1) & (RX_BUFFER_SIZE - 1);
}

void uart0Isr(void)
{
    if(UART0_RIS_R & UART_RIS_TXRIS)
    {
        if(UART0_FR_R & UART_FR_TXFE)
        {
            if(rxBufferWriteIndex != rxBufferReadIndex)
            {
                sendUIByte();
            }
            UART0_ICR_R = UART_ICR_TXIC;
        }
    }
}

//************************* NOTIFY USER AFTER PROCESSING DATA ********************************


//************************* SYSTEM RESET ********************************

void initiateSystemReset(void)
{
    NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
    while(true);
}

//************************* SYSTEM RESET ********************************


//************************* WRITING ADDRESS TO FLASH MEMORY ********************************

void eraseCurrentAddress(void)
{
    FLASH_FMC_R = getFlashKey() | FLASH_FMC_ERASE;
}

void setFlashAddress(void)
{
    FLASH_FMA_R = (uint32_t) FLASH_ADDRESS;
}

void setDeviceAddress(uint8_t address)
{
    FLASH_FMD_R = address;
}

void setInitialAddress(uint8_t address)
{
    setFlashAddress();
    eraseCurrentAddress();
    setDeviceAddress(address);
    writeAddress();
    currentDeviceAddress = address;
}

void writeAddress(void)
{
    FLASH_FMC_R = getFlashKey() | FLASH_FMC_WRITE;
}

bool changeDeviceAddress(uint8_t currentAddressUI, uint8_t newAddress)
{
    if(currentDeviceAddress != currentAddressUI)
    {
        return 0;
    }

    currentDeviceAddress = newAddress;
    setFlashAddress();                                              //loading fma with flash address
    eraseCurrentAddress();                                          //erase value in flash address using fmc
    setDeviceAddress(newAddress);                                   //store newAddress of device in flash address using fmd
    writeAddress();                                                 //write to fmc register
    return 1;
}

uint32_t getFlashKey(void)
{
    uint32_t flashKey;

    if(FLASH_BOOTCFG_R & FLASH_BOOTCFG_KEY)
    {
        flashKey = FLASH_FMC_WRKEY1;
    }
    else
    {
        flashKey = FLASH_FMC_WRKEY2;
    }

    return flashKey;
}

//************************* WRITING ADDRESS TO FLASH MEMORY ********************************
