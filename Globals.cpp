/*
 * Globals.cpp
 *
 * Created: 19.03.2017 09:24:37
 *  Author: Christof
 */
#define EXTERNALS_H_

#include "LuefterSteuerung.h"

//using namespace OneWire;
//using namespace RomCommands;


#define CQ "CQ"
#define C1 "C1"
#define Bd "Bd"


using namespace OneWire;
using namespace RomCommands;


const char *Node = NODE_STRING;
char IDNumber[12] EEMEM = "1784324-01";
char SerialNumber[12] EEMEM = "1958632254";
char IndexNumber[2] EEMEM = "A";
//TempSensor *activeSensor=NULL;

const char *luefterStatusStrings[4]={"aus","Stufe 1","Stufe 2","Auto"};

uint16_t actReportBetweenBlocks  = REPORT_BETWEEN_BLOCKS;
uint16_t actReportBetweenSensors = REPORT_BETWEEN_SENSORS;
uint16_t actWaitAfterLastSensor  = WAIT_AFTER_LAST_SENSOR;


volatile double fTemperatur=-999,fHumidity=-999,fDewPoint=-999,fAbsHumitdity=-999;
volatile uint8_t u8F1Swell=25,u8F1Hysterese=1,u8F2Swell=35,u8F2Hysterese=2;
volatile uint8_t u8FanSetStatus=0,u8FanActualStatus=0,u8FanActualStatusOld=FAN_STATUS_UNVALID;

uint8_t EEMEM ee_u8F1Swell=75,ee_u8F1Hysterese=5,ee_u8F2Swell=85,ee_u8F2Hysterese=5;
uint8_t EEMEM ee_u8FanSetStatus=FAN_STATUS_AUTO;


volatile uint8_t statusSensoren = KLIMASENSOR;
volatile uint8_t statusReport = TEMPREPORT;
volatile bool    sendStatusReport = false;
volatile uint8_t statusKlima = NOTHING_CLIMA_TODO;
volatile uint8_t statusLastSensor = NOTHING_LAST_TODO;

int errno;      // Globale Fehlerbehandlung

char SecurityLevel = 0;

//DS2484 owm;
//TempSensor *tempSensors[NUMBER_OF_TEMPSENSORS];
uint8_t actNumberSensors = 0;
/* Global variables for TWI */

volatile bool nextSendReady=false;

Communication cnet(0,Node,5,true);
ComReceiver cnetRec(&cnet,Node,cnetCommands,NUM_COMMANDS,NULL,0,NULL,NULL);

/* Global variables for TWI */
TWI_MasterDriver_t twiC_Master;    /*!< TWI master module. */
TWI_MasterDriver_t twiE_Master;    /*!< TWI master module. */

shtc3 humiSensor;

DS2484 owm;

SearchState searchState;
MultidropRomIterator selector(owm);

TempSensor *tempSensors[NUMBER_OF_TEMPSENSORS];
SENSINFOS storedSensors[NUMBER_STORED_SENSORS] EEMEM;
