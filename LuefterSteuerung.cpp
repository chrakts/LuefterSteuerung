/*
* LuefterSteuerung.cpp
*
* Created: 16.03.2017 13:03:01
* Author : a16007
*/

#include "LuefterSteuerung.h"

uint8_t doLastSensor();
uint8_t doClima();
void setup_twi();

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

  TWI_MasterInit(&twiC_Master, &TWIC, TWI_MASTER_INTLVL_LO_gc, TWI_BAUDSETTING);
  TWI_MasterInit(&twiE_Master, &TWIE, TWI_MASTER_INTLVL_LO_gc, TWI_BAUDSETTING);

	for(i=0;i<20;i++)
	{
		LED_GRUEN_TOGGLE;
		_delay_ms(50);
	}


	PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

	cnet.open(Serial::BAUD_57600,F_CPU);
}

int main(void)
{
uint8_t reportStarted = false;
	setup();

	init_mytimer();
 	setup_twi();

	uint8_t sensorReady=SENSOR_READY;
	MAX7328 maxTest(&twiE_Master,I2C_EXTENDER_ADDRESS);
  maxTest.newValue(0b11111111);

  humiSensor.begin(&twiC_Master);

	while(1)
  {
    RGBLED_ROT_ON;
    _delay_ms(1000);
    RGBLED_ROT_OFF;
    _delay_ms(1000);
    RGBLED_GRUEN_ON
    _delay_ms(1000);
    RGBLED_GRUEN_OFF
    _delay_ms(1000);
    RGBLED_BLAU_ON
    _delay_ms(1000);
    RGBLED_BLAU_OFF;
    _delay_ms(1000);
  }
	while (1)
	{
		cnetRec.comStateMachine();
		cnetRec.doJob();
		switch(statusSensoren)
		{
			case KLIMASENSOR:
				LED_ROT_ON;
				LED_GRUEN_OFF;
				sensorReady = doClima();
			break;
			case LASTSENSOR:
				LED_ROT_OFF;
				LED_GRUEN_ON;
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
                    sprintf(buffer,"%f",(double)fTemperatur);
                    cnet.sendStandard(buffer,BROADCAST,'C','1','t','F');
                break;
                case HUMIREPORT:
                    sprintf(buffer,"%f",(double)fHumidity);
                    cnet.sendStandard(buffer,BROADCAST,'C','1','h','F');
                break;
                case ABSHUMIREPORT:
                    sprintf(buffer,"%f",(double)fAbsHumitdity);
                    cnet.sendStandard(buffer,BROADCAST,'C','1','a','F');
                break;
                case DEWPOINTREPORT:
                    sprintf(buffer,"%f",(double)fDewPoint);
                    cnet.sendStandard(buffer,BROADCAST,'C','1','d','F');
                break;
                case L1SWELLREPORT:
                    sprintf(buffer,"%ud",(uint8_t)u8F1Swell);
                    cnet.sendStandard(buffer,BROADCAST,'L','1','L','F');
                break;
                case L2SWELLREPORT:
                    sprintf(buffer,"%ud",(uint8_t)u8F2Swell);
                    cnet.sendStandard(buffer,BROADCAST,'L','1','G','F');
                break;
                case L1HYSTREPORT:
                    sprintf(buffer,"%ud",(uint8_t)u8F1Hysterese);
                    cnet.sendStandard(buffer,BROADCAST,'L','1','H','F');
                break;
                case L2HYSTREPORT:
                    sprintf(buffer,"%ud",(uint8_t)u8F2Hysterese);
                    cnet.sendStandard(buffer,BROADCAST,'L','1','I','F');
                break;
                case LASTREPORT:
                    MyTimers[TIMER_REPORT].value = actReportBetweenBlocks;
                    MyTimers[TIMER_REPORT].state = TM_START;
                break;
            }
        }
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
static unsigned char crcSH11;
bool noError;
static unsigned int iTemperature,iHumidity;
	switch(statusKlima)
	{
		case NOTHING_CLIMA_TODO:
			statusKlima = START_TCONVERSION;
		break;
		case START_TCONVERSION: // Durchlaufzeit ca. 55 탎
			LED_ROT_ON;
			crcSH11 = 0;
			noError=humiSensor.startMeasure(); // startConversion(0,&crcSH11);
			if (noError==true)
			{
				statusKlima = WAIT_TCONVERSION;
				MyTimers[TIMER_TEMPERATURE].value = 44; //22
				MyTimers[TIMER_TEMPERATURE].state = TM_START;
			}
			else
				statusKlima = NOTHING_CLIMA_TODO;
		break;
		/*
		case READ_TCONVERSION:  // Durchlaufzeit ca. 82 탎
			noError = readConversion(&iTemperature,&crcSH11);
			if (noError==true)
			{
				statusKlima = START_HCONVERSION;
			}
			else
				statusKlima = NOTHING_CLIMA_TODO;
		break;
		case START_HCONVERSION:
			noError=startConversion(1,&crcSH11);
			if (noError==0)
			{
				statusKlima = WAIT_HCONVERSION;
				MyTimers[TIMER_TEMPERATURE].value = 14; // 7
				MyTimers[TIMER_TEMPERATURE].state = TM_START;
			}
			else
				statusKlima = NOTHING_CLIMA_TODO;
		break;
		case READ_HCONVERSION:
			noError = readConversion(&iHumidity,&crcSH11);
			if (noError==0)
			{
				statusKlima = CALC_CONVERSION1;
			}
			else
				statusKlima = NOTHING_CLIMA_TODO;
		break;
		case CALC_CONVERSION1:  // Durchlaufzeit ca. 58 탎
			fHumidity = (float)iHumidity;
			fTemperatur = (float)iTemperature;
			calc_sth11(&fHumidity ,&fTemperatur);
			statusKlima = CALC_CONVERSION2;
		break;
		case CALC_CONVERSION2:  // Durchlaufzeit ca. 141
			fDewPoint = calc_dewpoint_float(fHumidity,fTemperatur); // calculate dew point temperature
			statusKlima = CALC_CONVERSION3;
		break;
		case CALC_CONVERSION3:  // Durchlaufzeit ca. 230 탎
			fAbsHumitdity = abs_feuchte(fHumidity,fTemperatur); // calculate dew point temperature
			statusKlima = NOTHING_CLIMA_TODO;
      LED_ROT_OFF;
		break;*/
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

