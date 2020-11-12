/*
 * External.h
 *
 * Created: 03.04.2017 21:04:41
 *  Author: Christof
 */



#ifndef EXTERNAL_H_
#define EXTERNAL_H_

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "CRC_Calc.h"
#include "Communication.h"
#include "Masters/Masters.h"
#include "Slaves/Slaves.h"
#include "RomId/RomCommands.h"
#include "RomId/RomId.h"
#include "Masters/DS248x/DS2484/DS2484.h"
#include "twi_master_driver.h"
#include "shtc3.h"
#include "TempSensor.h"

#include "myTimers.h"

using namespace OneWire;
using namespace RomCommands;


extern volatile uint8_t UART0_ring_received;
extern volatile  char UART0_ring_buffer[UART0_RING_BUFFER_SIZE];
extern volatile uint8_t UART1_ring_received;
extern volatile  char UART1_ring_buffer[UART1_RING_BUFFER_SIZE];

extern char const *Node;
extern char IDNumber[12] EEMEM ;
extern char SerialNumber[12] EEMEM;
extern char IndexNumber[2] EEMEM;

extern uint16_t actReportBetweenBlocks;
extern uint16_t actReportBetweenSensors;
extern uint16_t actWaitAfterLastSensor;

volatile extern float fTemperatur,fHumidity,fDewPoint,fAbsHumitdity;
volatile extern uint8_t u8F1Swell,u8F1Hysterese,u8F2Swell,u8F2Hysterese;

extern const char *fehler_text[];
extern uint8_t actNumberSensors;

extern volatile uint8_t statusReport;
extern volatile bool sendStatusReport;

extern volatile uint8_t statusKlima;
extern volatile uint8_t statusDruck;
extern volatile uint8_t statusSensoren;
extern volatile uint8_t statusLicht;
extern volatile uint8_t statusLastSensor;

extern char SecurityLevel;

class Communication;   // Forward declaration
class ComReceiver;
extern Serial debug;
extern Communication cnet;
extern ComReceiver cnetRec;
extern CRC_Calc crcGlobal;

extern volatile uint8_t sendFree;
extern volatile bool nextSendReady;

/* Global variables for TWI */
extern TWI_MasterDriver_t twiC_Master;    /*!< TWI master module. */
extern TWI_MasterDriver_t twiE_Master;    /*!< TWI master module. */

extern shtc3 humiSensor;

extern DS2484 owm;
extern SearchState searchState;
extern MultidropRomIterator selector;

extern TempSensor *tempSensors[NUMBER_OF_TEMPSENSORS];
extern SENSINFOS storedSensors[NUMBER_STORED_SENSORS] EEMEM;


#endif /* EXTERNAL_H_ */
