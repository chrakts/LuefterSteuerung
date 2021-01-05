/*
 * CommandFunctions.cpp
 *
 * Created: 26.04.2017 14:54:45
 *  Author: a16007
 */

#include "CommandFunctions.h"
#include "External.h"
#include "../Secrets/secrets.h"


COMMAND cnetCommands[NUM_COMMANDS] =
	{
    cmultiStandardCommands,
		{'P','i',CUSTOMER,NOPARAMETER,0,jobGetIDNumber},
		{'P','s',CUSTOMER,NOPARAMETER,0,jobGetSerialNumber},
		{'P','x',CUSTOMER,NOPARAMETER,0,jobGetIndex},
		{'P','I',PRODUCTION,STRING,13,jobSetIDNumber},
		{'P','S',PRODUCTION,STRING,13,jobSetSerialNumber},
		{'P','X',PRODUCTION,STRING,3,jobSetIndexNumber},
		{'C','t',CUSTOMER,NOPARAMETER,0,jobGetCTemperatureSensor},
		{'C','h',CUSTOMER,NOPARAMETER,0,jobGetCHumiditySensor},
		{'C','d',CUSTOMER,NOPARAMETER,0,jobGetCDewPointSensor},
		{'C','a',CUSTOMER,NOPARAMETER,0,jobGetCAbsHumiditySensor},
		{'L','a',CUSTOMER,NOPARAMETER,0,jobGetLuefterActualStatus},
		{'L','s',CUSTOMER,NOPARAMETER,0,jobGetLuefterSetStatus},
		{'L','L',CUSTOMER,UINT_8,1,jobSetLuefter1OnValue},
		{'L','G',CUSTOMER,UINT_8,1,jobSetLuefter2OnValue},
		{'L','H',CUSTOMER,UINT_8,1,jobSetLuefter1HystValue},
		{'L','I',CUSTOMER,UINT_8,1,jobSetLuefter2HystValue},
		{'L','S',CUSTOMER,STRING,8,jobSetLuefterSetStatus},
		{'L','l',CUSTOMER,NOPARAMETER,0,jobGetLuefter1OnValue},
		{'L','g',CUSTOMER,NOPARAMETER,0,jobGetLuefter2OnValue},
		{'L','h',CUSTOMER,NOPARAMETER,0,jobGetLuefter1HystValue},
		{'L','i',CUSTOMER,NOPARAMETER,0,jobGetLuefter2HystValue},
		{'T','B',CUSTOMER,UINT_16,1,jobSetTimeBetweenBlocks},
		{'T','S',CUSTOMER,UINT_16,1,jobSetTimeBetweenSensors},
		{'T','W',CUSTOMER,UINT_16,1,jobWaitAfterLastSensor}
	};



void jobGetLuefter1OnValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	comRec->sendAnswerInt(function,address,job,u8F1Swell,true);
}

void jobGetLuefter2OnValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	comRec->sendAnswerInt(function,address,job,u8F2Swell,true);
}

void jobGetLuefter1HystValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	comRec->sendAnswerInt(function,address,job,u8F1Hysterese,true);
}

void jobGetLuefter2HystValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	comRec->sendAnswerInt(function,address,job,u8F2Hysterese,true);
}

void jobGetLuefterSetStatus(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
  cnet.sendStandard(luefterStatusStrings[u8FanSetStatus],BROADCAST,'L','1','s',true);
}

void jobGetLuefterActualStatus(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
  comRec->sendAnswer(luefterStatusStrings[u8FanActualStatus],'L','1','a',true);
}

void jobSetLuefter1OnValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	u8F1Swell = ( (uint8_t*) pMem )[0];
	MyTimers[TIMER_SAVE_DELAY].state = TM_START;
	comRec->Getoutput()->broadcastUInt8(u8F1Swell,function,address,job);
}

void jobSetLuefter2OnValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	u8F2Swell = ( (uint8_t*) pMem )[0];
	MyTimers[TIMER_SAVE_DELAY].state = TM_START;
	comRec->Getoutput()->broadcastUInt8(u8F2Swell,function,address,job);
}

void jobSetLuefter1HystValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	u8F1Hysterese = ( (uint8_t*) pMem )[0];
	MyTimers[TIMER_SAVE_DELAY].state = TM_START;
	comRec->Getoutput()->broadcastUInt8(u8F1Hysterese,function,address,job);
}

void jobSetLuefter2HystValue(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	u8F2Hysterese = ( (uint8_t*) pMem )[0];
	MyTimers[TIMER_SAVE_DELAY].state = TM_START;
	comRec->Getoutput()->broadcastUInt8(u8F2Hysterese,function,address,job);
}

void jobSetLuefterSetStatus(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
char *temp;
	temp = (char*) pMem;
	switch(temp[0])
	{
    case 'a':
      u8FanSetStatus = FAN_STATUS_OFF;
    break;
    case 'S':
      if( temp[6]=='1' )
        u8FanSetStatus = FAN_STATUS_1;
      else
        u8FanSetStatus = FAN_STATUS_2;
    break;
    case 'A':
      u8FanSetStatus = FAN_STATUS_AUTO;
    break;
	}
	reportFanSetStatus(comRec->Getoutput());
	//comRec->Getoutput()->broadcastUInt8(u8FanSetStatus,function,address,job);
}

void jobSetIDNumber(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	if (strlen((char*)pMem)<=11)
	{
		eeprom_write_block((char*)pMem,IDNumber,strlen((char*)pMem)+1);
		comRec->sendPureAnswer(function,address,job,true);
	}
	else
		comRec->sendPureAnswer(function,address,job,false);
}

void jobSetSerialNumber(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	if (strlen((char*)pMem)<=11)
	{
		eeprom_write_block((char*)pMem,SerialNumber,strlen((char*)pMem)+1);
		comRec->sendPureAnswer(function,address,job,true);
	}
	else
	comRec->sendPureAnswer(function,address,job,false);
}

void jobSetIndexNumber(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	if (strlen((char*)pMem)<=2)
	{
		eeprom_write_block((char*)pMem,IndexNumber,strlen((char*)pMem)+1);
		comRec->sendPureAnswer(function,address,job,true);
	}
	else
		comRec->sendPureAnswer(function,address,job,false);
}

void jobGetIDNumber(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	char temp[12];
	eeprom_read_block(temp,IDNumber,12);
	comRec->sendAnswer(temp,function,address,job,true);
}

void jobGetSerialNumber(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	char temp[12];
	eeprom_read_block(temp,SerialNumber,12);
	comRec->sendAnswer(temp,function,address,job,true);
}

void jobGetIndex(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	char temp[2];
	eeprom_read_block(temp,IndexNumber,2);
	comRec->sendAnswer(temp,function,address,job,true);
}

void jobGetCTemperatureSensor(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
char answer[20]="";

	sprintf(answer,"%f",(double)fTemperatur);
	comRec->sendAnswer(answer,function,address,job,true);
}

void jobGetCHumiditySensor(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	char answer[20]="";

	sprintf(answer,"%f",(double)fHumidity);
	comRec->sendAnswer(answer,function,address,job,true);
}

void jobGetCAbsHumiditySensor(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	char answer[20]="";

	sprintf(answer,"%f",(double)fAbsHumitdity);
	comRec->sendAnswer(answer,function,address,job,true);
}

void jobGetCDewPointSensor(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	char answer[20]="";

	sprintf(answer,"%f",(double)fDewPoint);
	comRec->sendAnswer(answer,function,address,job,true);
}

void jobSetTimeBetweenBlocks(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	actReportBetweenBlocks = ( (uint16_t*) pMem )[0];
	comRec->sendAnswerInt(function,address,job,actReportBetweenBlocks,true);
}

void jobSetTimeBetweenSensors(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	actReportBetweenSensors = ( (uint16_t*) pMem )[0];
	comRec->sendAnswerInt(function,address,job,actReportBetweenSensors,true);
}

void jobWaitAfterLastSensor(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
	actWaitAfterLastSensor = ( (uint16_t*) pMem )[0];
	comRec->sendAnswerInt(function,address,job,actWaitAfterLastSensor,true);
}

void reportFanSetStatus(Communication *com)
{
  com->sendStandard(luefterStatusStrings[u8FanSetStatus],BROADCAST,'L','1','s','T');
}

void reportFanActualStatus(Communication *com)
{
  com->sendStandard(luefterStatusStrings[u8FanActualStatus],BROADCAST,'L','1','a','T');
}
