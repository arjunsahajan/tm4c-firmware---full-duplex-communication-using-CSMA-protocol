/*
 * funDef.h
 *
 *  Created on: 02-Nov-2021
 *      Author: arjun
 */

#ifndef FUNDEF_H_
#define FUNDEF_H_

#define COMMAND_SET                 0x00
#define COMMAND_RGB                 0x48
#define COMMAND_RESET_NODE          0x7F
#define COMMAND_CHANGE_ADDRESS_NODE 0x7A
#define COMMAND_PULSE               0x02
#define COMMAND_SQUARE              0x03
#define COMMAND_POLL                0x78
#define COMMAND_POLL_RESPONSE       0x79
#define COMMAND_GET                 0x30
#define COMMAND_DATA_REPORT         0x31
#define COMMAND_ACK_RESPOSNE        0x70
#define COMMAND_ACK_CHECK           0x7F

#define MAX_CHARS         80
#define MAX_FIELDS        8
#define DEBUG             1
#define DATA_SIZE         8
#define FLASH_ADDRESS     ((volatile uint8_t*) 0x00020000)
#define TX_MSG_ARRAY_SIZE 32
#define RX_BUFFER_SIZE    64
#define MAX_RE_TX_COUNT   5
#define MIN_BACK_OFF_TIME 10
#define TX_LED_TIMEOUT    10
#define RX_LED_TIMEOUT    10
#define RE_TX_MULTIPLIER  10

#define TX_ENABLE         (*((volatile uint32_t *) (0x42000000 + (0x400053FC - 0x40000000) * 32 + 5 * 4)))
#define GREEN_LED         (*((volatile uint32_t *) (0x42000000 + (0x400243FC - 0x40000000) * 32 + 4 * 4)))
#define RED_LED           (*((volatile uint32_t *) (0x42000000 + (0x400243FC - 0x40000000) * 32 + 5 * 4)))
#define RED_LED_ONBOARD   (*((volatile uint32_t *) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 1 * 4)))
#define BLUE_LED_ONBOARD  (*((volatile uint32_t *) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 2 * 4)))
#define GREEN_LED_ONBOARD (*((volatile uint32_t *) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 3 * 4)))
#define PB1               (*((volatile uint32_t *) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 4 * 4)))

#define GREEN_LED_MASK         16
#define RED_LED_MASK           32
#define RED_LED_ONBOARD_MASK   2
#define BLUE_LED_ONBOARD_MASK  4
#define GREEN_LED_ONBOARD_MASK 8
#define PB1_MASK               16
#define AIN3_MASK              1

#define TX_ENABLE_MASK 32
#define TX_MASK        2
#define RX_MASK        1

uint8_t  currentDeviceAddress;
int8_t   msgInProgress;
uint8_t  msgTXphase;
uint8_t  msgRXphase;
uint8_t  readIndexMsgFIFO;
uint8_t  writeIndexMsgFIFO;
char     rxBuffer[RX_BUFFER_SIZE];
uint8_t  rxBufferReadIndex;
uint8_t  rxBufferWriteIndex;
uint16_t raw;
uint32_t txLEDtimeout;
uint32_t rxLEDtimeout;
uint8_t  testCS;
uint8_t  seed;

bool csBit;
bool randomBit;
bool ackBit;
bool busyBit;
bool testDone;
bool pulseBit;
bool squareBit;
bool itsTimeToSend;

typedef struct _ActionWave
{
    uint16_t time1;
    uint8_t  level1;
    uint16_t time2;
    uint8_t  level2;
    uint16_t count;
    bool     valid;
    uint8_t  phase;
    uint8_t  deltaT;
} ActionWave;

ActionWave wave;

typedef struct _RX485Msg
{
    uint8_t dstAddress;
    uint8_t srcAddress;
    uint8_t command;
    uint8_t channel;
    uint8_t sequenceID;
    uint8_t size;
    uint8_t data[DATA_SIZE];
    uint8_t checksum;
} RX485Msg;

RX485Msg rxMsg;

typedef struct _USER_DATA
{
    char    buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char    fieldType[MAX_FIELDS];
} USER_DATA;

typedef struct _TX485Msg
{
    uint16_t dstAddress;
    uint8_t  srcAddress;
    uint8_t  command;
    uint8_t  channel;
    uint8_t  size;
    uint8_t  sequenceID;
    uint8_t  data[DATA_SIZE];
    uint8_t  checksum;
    uint8_t  timeToTransmit;
    uint8_t  countTX;
    bool     valid;
} TX485Msg;

TX485Msg TX485MsgsArray[TX_MSG_ARRAY_SIZE];

void     initHw(void);
void     initSw(void);
void     initUart1(void);
void     initUart0(void);
void     getsUart0(USER_DATA*);
void     parseFields(USER_DATA*);
bool     isCommand(USER_DATA*, const char*, uint8_t);
uint32_t getFieldInteger(USER_DATA*, uint8_t);
char*    getFieldString(USER_DATA*, uint8_t);
uint8_t  stringToNum(const char*);
bool     strCompare(const char*, const char*);
uint8_t  strLength(const char*);
char*    reverseNum(uint8_t);
void     numToString(uint8_t);
uint8_t  countArguments(USER_DATA* dataPtr, uint8_t);
void     initiateSystemReset(void);
bool     changeDeviceAddress(uint8_t, uint8_t);
uint32_t getFlashKey(void);
void     writeAddress(void);
void     setInitialAddress(uint8_t);
void     setDeviceAddress(uint8_t address);
void     setFlashAddress(void);
void     eraseCurrentAddress(void);
void     sendRS485(uint8_t, uint8_t, uint8_t, uint8_t [], uint8_t, bool);
void     sendRS485Byte(void);
void     userInterfaceTx(char*);
uint8_t  calculateChecksum(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t []);
void     processData(RX485Msg*);
void     sendUIByte(void);
uint8_t  calculatePower(uint8_t, uint8_t);
void     initAdc0Ss3(void);
void     setAdc0Ss3Mux(uint8_t);
void     setAdc0Ss3Log2AverageCount(uint8_t);
int16_t  readAdc0Ss3(void);
void     initPWM(void);

#endif /* FUNDEF_H_ */
