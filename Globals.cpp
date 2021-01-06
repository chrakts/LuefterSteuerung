/*
 * Globals.cpp
 *
 * Created: 19.03.2017 09:24:37
 *  Author: Christof
 */
#define EXTERNALS_H_

#include "LuefterSteuerung.h"

#define LB "LB"

const char *Node = NODE_STRING;
char IDNumber[12] EEMEM = "1784324-01";
char SerialNumber[12] EEMEM = "1958632254";
char IndexNumber[2] EEMEM = "A";

const char *luefterStatusStrings[4]={"aus","Stufe 1","Stufe 2","Auto"};

uint16_t actReportBetweenBlocks  = REPORT_BETWEEN_BLOCKS;
uint16_t actReportBetweenSensors = REPORT_BETWEEN_SENSORS;
uint16_t actWaitAfterLastSensor  = WAIT_AFTER_LAST_SENSOR;


volatile double fTemperatur=-999,fHumidity=-999,fDewPoint=-999,fAbsHumitdity=-999;
volatile uint8_t u8F1Swell=25,u8F1Hysterese=1,u8F2Swell=35,u8F2Hysterese=2;
volatile uint8_t u8FanSetStatus=FAN_STATUS_AUTO,u8FanActualStatus=FAN_STATUS_2,u8FanActualStatusOld=FAN_STATUS_UNVALID;

volatile uint8_t u8oldF1Swell,u8oldF1Hysterese,u8oldF2Swell,u8oldF2Hysterese,u8oldFanSetStatus;

uint8_t EEMEM ee_u8F1Swell=75,ee_u8F1Hysterese=5,ee_u8F2Swell=85,ee_u8F2Hysterese=5;
uint8_t EEMEM ee_u8FanSetStatus=FAN_STATUS_AUTO;


volatile uint8_t statusSensoren = KLIMASENSOR;
volatile uint8_t statusReport = TEMPREPORT;
volatile bool    sendStatusReport = false;
volatile uint8_t statusKlima = NOTHING_CLIMA_TODO;
volatile uint8_t statusLastSensor = NOTHING_LAST_TODO;

int errno;      // Globale Fehlerbehandlung

char SecurityLevel = 0;

uint8_t actNumberSensors = 0;

volatile bool nextSendReady=false;

Communication cnet(0,Node,5,true);
ComReceiver cnetRec(&cnet,Node,cnetCommands,NUM_COMMANDS,NULL,0,NULL,NULL);

/* Global variables for TWI */
TWI_MasterDriver_t twiC_Master;    /*!< TWI master module. */
TWI_MasterDriver_t twiE_Master;    /*!< TWI master module. */

shtc3 humiSensor;

