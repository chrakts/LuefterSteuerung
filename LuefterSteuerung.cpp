/*
* LuefterSteuerung.cpp
*
* Created: 16.03.2017 13:03:01
* Author : a16007
*/

#include "LuefterSteuerung.h"

void setup()
{
  init_clock(SYSCLK,PLL,true,CLOCK_CALIBRATION);
	PORTA_DIRSET = PIN2_bm | PIN3_bm | PIN4_bm;
	PORTA_OUTSET = 0xff;

	PORTB_DIRSET = 0xff;

	PORTC_DIRSET = PIN1_bm;

	PORTD_DIRSET = PIN0_bm | PIN4_bm | PIN5_bm | PIN7_bm;
	PORTD_DIRCLR = PIN6_bm;
	PORTD_OUTCLR = PIN4_bm | PIN5_bm;

	PORTE_DIRSET = 0xff;

	uint8_t i;

 // TWI_MasterInit(&twiC_Master, &TWIC, TWI_MASTER_INTLVL_LO_gc, TWI_BAUDSETTING);
  TWI_MasterInit(&twiE_Master, &TWIE, TWI_MASTER_INTLVL_LO_gc, TWI_BAUDSETTING);

	for(i=0;i<20;i++)
	{
		LED_GRUEN_TOGGLE;
		_delay_ms(50);
	}
  LEDGRUEN_OFF;

	PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

	cnet.open(Serial::BAUD_57600,F_CPU);
	cnet.sendInfo("Hello Again","BR");


}

int main(void)
{
uint8_t reportStarted = false;
bool test;
	setup();

	cnet.broadcastUInt8((uint8_t) RST.STATUS,'S','0','R');

  readEEData();

	init_mytimer();
// 	setup_twi();

	uint8_t sensorReady=SENSOR_READY;
	/*
	MAX7328 maxTest(&twiE_Master,I2C_EXTENDER_ADDRESS);
  maxTest.newValue(0b11111111);
*/
  humiSensor.begin(&twiE_Master);
  humiSensor.setMode(SHTC3_NORMAL_T_FIRST);
  _delay_ms(10);
  cnet.broadcastUInt16(humiSensor.getID(),'a','b','c');

	while (1)
	{
		cnetRec.comStateMachine();
		cnetRec.doJob();
		if(u8FanSetStatus!=FAN_STATUS_AUTO)
      u8FanActualStatus = u8FanSetStatus;
    else // FAN = Auto
    {
      uint8_t u8Humi = (uint8_t)fHumidity;
      switch(u8FanActualStatus)
      {
        case FAN_STATUS_OFF:
          if( u8Humi > u8F1Swell)
            u8FanActualStatus = FAN_STATUS_1;
        break;
        case FAN_STATUS_1:
          if( u8Humi < u8F1Swell-u8F1Hysterese)
            u8FanActualStatus = FAN_STATUS_OFF;
          else if( u8Humi > u8F2Swell )
            u8FanActualStatus = FAN_STATUS_2;
        break;
        case FAN_STATUS_2:
          if( u8Humi < u8F2Swell-u8F2Hysterese)
            u8FanActualStatus = FAN_STATUS_1;
        break;
      }
    }
    if( u8FanActualStatusOld != u8FanActualStatus)
    {
      reportFanActualStatus(&cnet);
      u8FanActualStatusOld=u8FanActualStatus;
    }

		switch(statusSensoren)
		{
			case KLIMASENSOR:
				LED_ROT_ON;
				sensorReady = doClima();
			break;
			case LASTSENSOR:
				LED_ROT_OFF;
				sensorReady = doLastSensor();
			break;
		}
		if (sensorReady==SENSOR_READY)
		{
			statusSensoren++;
			if (statusSensoren>LASTSENSOR)
			{
				statusSensoren = KLIMASENSOR;
				if(reportStarted==false)
                {
                    MyTimers[TIMER_REPORT].state = TM_START;
                    reportStarted = true;
                }
			}
		}
		if( sendStatusReport )
    {
        char buffer[16];
        sendStatusReport = false;
        MyTimers[TIMER_REPORT].value = actReportBetweenSensors;
        MyTimers[TIMER_REPORT].state = TM_START;
        switch(statusReport)
        {
            case TEMPREPORT:
                sprintf(buffer,"%.1f",(double)fTemperatur);
                cnet.sendStandard(buffer,BROADCAST,'C','1','t','F');
            break;
            case HUMIREPORT:
                sprintf(buffer,"%.1f",(double)fHumidity);
                cnet.sendStandard(buffer,BROADCAST,'C','1','h','F');
            break;
            case ABSHUMIREPORT:
                sprintf(buffer,"%.1f",(double)fAbsHumitdity);
                cnet.sendStandard(buffer,BROADCAST,'C','1','a','F');
            break;
            case DEWPOINTREPORT:
                sprintf(buffer,"%.1f",(double)fDewPoint);
                cnet.sendStandard(buffer,BROADCAST,'C','1','d','F');
            break;
            case L1SWELLREPORT:
                sprintf(buffer,"%u",(uint8_t)u8F1Swell);
                cnet.sendStandard(buffer,BROADCAST,'L','1','L','F');
            break;
            case L2SWELLREPORT:
                sprintf(buffer,"%u",(uint8_t)u8F2Swell);
                cnet.sendStandard(buffer,BROADCAST,'L','1','G','F');
            break;
            case L1HYSTREPORT:
                sprintf(buffer,"%u",(uint8_t)u8F1Hysterese);
                cnet.sendStandard(buffer,BROADCAST,'L','1','H','F');
            break;
            case L2HYSTREPORT:
                sprintf(buffer,"%u",(uint8_t)u8F2Hysterese);
                cnet.sendStandard(buffer,BROADCAST,'L','1','I','F');
            break;
            case FANACTUALSTATUSREPORT:
                reportFanActualStatus(&cnet);
            break;
            case FANSETSTATUSREPORT:
                reportFanSetStatus(&cnet);
                //cnet.sendStandard(luefterStatusStrings[u8FanSetStatus],BROADCAST,'L','1','s','F');
            break;
            case LASTREPORT:
                MyTimers[TIMER_REPORT].value = actReportBetweenBlocks;
                MyTimers[TIMER_REPORT].state = TM_START;
            break;
        }
    }
    else
      ;
	}
}

uint8_t doLastSensor()
{
	switch( statusLastSensor )
	{
		case NOTHING_LAST_TODO:
			MyTimers[TIMER_TEMPERATURE].value = actWaitAfterLastSensor;
			MyTimers[TIMER_TEMPERATURE].state = TM_START;
			statusLastSensor = WAIT_LAST;
		break;
		case READY_LAST:
			statusLastSensor = NOTHING_LAST_TODO;
		break;
	}
	return statusLastSensor;
}

uint8_t doClima()
{
bool noError;

	switch(statusKlima)
	{
		case NOTHING_CLIMA_TODO:
			statusKlima = WAKEUP;
		break;
		case WAKEUP:
		  humiSensor.wakeup();
		  statusKlima = WAIT_WAKEUP;
      MyTimers[TIMER_TEMPERATURE].value = 2; //22
      MyTimers[TIMER_TEMPERATURE].state = TM_START;
    break;

		case START_CONVERSION: //
			LED_ROT_ON;

			noError=humiSensor.startMeasure();
			if (noError==true)
			{
				statusKlima = WAIT_CONVERSION;
				MyTimers[TIMER_TEMPERATURE].value = 2; //22
				MyTimers[TIMER_TEMPERATURE].state = TM_START;
			}
			else
				statusKlima = NOTHING_CLIMA_TODO;
		break;

		case READ_CONVERSION:  // Durchlaufzeit ca.
			noError = humiSensor.readResults();
			if (noError==true)
			{
				statusKlima = WAIT_READ;
				MyTimers[TIMER_TEMPERATURE].value = 2; //22
				MyTimers[TIMER_TEMPERATURE].state = TM_START;
			}
			else
				statusKlima = NOTHING_CLIMA_TODO;
		break;
		case CALC_CONVERSION0:  // Durchlaufzeit ca.
		  humiSensor.getResults(fTemperatur,fHumidity);
		  humiSensor.sleep();
			statusKlima = CALC_CONVERSION1;
		break;
		case CALC_CONVERSION1:  // Durchlaufzeit ca.
		  fAbsHumitdity =  humiSensor.calcAbsHumi(fTemperatur,fHumidity);
			statusKlima = CALC_CONVERSION2;
		break;
		case CALC_CONVERSION2:  // Durchlaufzeit ca.
		  fDewPoint =  humiSensor.calcDewPoint(fTemperatur,fHumidity);
			statusKlima = NOTHING_CLIMA_TODO;
		break;
	}
	return(statusKlima);
}

void setup_twi()
{
  char romBuf[40];
  bool last_dev = false;

	OneWireMaster::CmdResult result = owm.begin(&twiC_Master,0x18);
	cnet.sendInfo("Master Ready",BROADCAST);
	if(result != OneWireMaster::Success)
	{
        cnet.sendAlarm("Failed 1W Master",BROADCAST);
		while(1);
	}
	result = owm.OWReset();
	if(result == OneWireMaster::Success)
	{
		result = OWFirst(owm, searchState);
		if(result == OneWireMaster::Success)
		{
			uint8_t temp_index = 0;
			do
			{
				cnet.sendInfo("Search sensor: ",BROADCAST);
				last_dev = searchState.last_device_flag;
				if( (searchState.romId.familyCode() == 0x28) | (searchState.romId.familyCode() == 0x10))
				{
					if (actNumberSensors<NUMBER_OF_TEMPSENSORS)
					{
						tempSensors[actNumberSensors] = new TempSensor(selector,true,temp_index);
						temp_index++;
						tempSensors[actNumberSensors]->setRomID(searchState.romId);
						buffer_rom_id(romBuf,searchState.romId);
						cnet.sendInfo(romBuf,BROADCAST);
						actNumberSensors++;
					}
					else
					{
						cnet.sendWarning("Too much sensors",BROADCAST);
					}
				}
				result = OWNext(owm, searchState);
			}
			while( (result == OneWireMaster::Success) && (last_dev==false) );
		}
		else
		{
			//cnet.print("OWFirst failed with error code: ");
			//cnet.println(result, Serial::DEC);
		}
	}
	else
	{
		cnet.println("No 1-wire devices");
	}
	sprintf(romBuf,"No. Sensoren:%d", actNumberSensors);
	cnet.sendInfo(romBuf,BROADCAST);
}

//*********************************************************************
void buffer_rom_id(char *buffer,OneWire::RomId & romId)
{
	char temp[3];
	strcpy(buffer,"0x");
	for(uint8_t idx = 0; idx < RomId::byteLen; idx++)
	{
		sprintf(temp,"%x",romId[idx]);
		strcat(buffer,temp);
	}
}

void readEEData()
{
  u8F1Swell = eeprom_read_byte(&ee_u8F1Swell);
  u8F1Hysterese = eeprom_read_byte(&ee_u8F1Hysterese);
  u8F2Swell = eeprom_read_byte(&ee_u8F2Swell);
  u8F2Hysterese= eeprom_read_byte(&ee_u8F2Hysterese);
  u8FanSetStatus= eeprom_read_byte(&ee_u8FanSetStatus);
}


