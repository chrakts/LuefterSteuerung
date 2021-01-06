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
  TWI_MasterInit(&twiC_Master, &TWIC, TWI_MASTER_INTLVL_LO_gc, TWI_BAUDSETTING);

	for(i=0;i<20;i++)
	{
		LEDGRUEN_TOGGLE;
		_delay_ms(50);
	}
  LEDGRUEN_OFF;

	PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

	cnet.open(Serial::BAUD_57600,F_CPU);
}

int main(void)
{
uint8_t reportStarted = false;

	setup();

	cnet.broadcastUInt8((uint8_t) RST.STATUS,'S','0','R');

  readEEData();

	init_mytimer();

	uint8_t sensorReady=SENSOR_READY;
	MAX7328 maxTest(&twiE_Master,I2C_EXTENDER_ADDRESS);
  maxTest.newValue(0xff);
  while(!TWI_MasterReady(&twiE_Master))
    ;
  humiSensor.begin(&twiC_Master);
  humiSensor.setMode(SHTC3_NORMAL_T_FIRST);
  cnet.broadcastUInt16(humiSensor.getID(),'S','I','D');

	while (1)
	{
		cnetRec.comStateMachine();
		cnetRec.doJob();
		// Ermittlung des neuen Lüfterstatus
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
    // Setzen der Relais und der LEDs entsprechend des Lüfterstatus
    uint8_t outputs = 0xff;
    switch(u8FanActualStatus)
    {
      case FAN_STATUS_OFF:
        outputs ^= LED_RGB_RED;
      break;
      case FAN_STATUS_1:
        outputs ^= LED_RGB_GREEN | POWER_2;
      break;
      case FAN_STATUS_2:
        outputs ^= LED_RGB_BLUE| POWER_3;
      break;
    }
    maxTest.updateValue(outputs);

    // Falls sich der Lüfterstatus geändert, wird dieser gesendet
    if( u8FanActualStatusOld != u8FanActualStatus)
    {
      reportFanActualStatus(&cnet);
      u8FanActualStatusOld=u8FanActualStatus;
    }

		switch(statusSensoren)
		{
			case KLIMASENSOR:
				sensorReady = doClima();
			break;
			case LASTSENSOR:
				LEDROT_OFF;
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
                LEDGRUEN_ON;
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
                LEDGRUEN_OFF;
                MyTimers[TIMER_REPORT].value = actReportBetweenBlocks;
                MyTimers[TIMER_REPORT].state = TM_START;
            break;
        }
    }

    if( (u8oldF1Swell      != u8F1Swell     ) |
        (u8oldF1Hysterese  != u8F1Hysterese ) |
        (u8oldF2Swell      != u8F2Swell     ) |
        (u8oldF2Hysterese  != u8F2Hysterese ) |
        (u8oldFanSetStatus != u8FanSetStatus)
      )
    {
      writeEEData();
      u8oldF1Swell      = u8F1Swell     ;
      u8oldF1Hysterese  = u8F1Hysterese ;
      u8oldF2Swell      = u8F2Swell     ;
      u8oldF2Hysterese  = u8F2Hysterese ;
      u8oldFanSetStatus = u8FanSetStatus;
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
			LEDROT_ON;

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

void readEEData()
{
  u8F1Swell      = eeprom_read_byte(&ee_u8F1Swell);
  u8F1Hysterese  = eeprom_read_byte(&ee_u8F1Hysterese);
  u8F2Swell      = eeprom_read_byte(&ee_u8F2Swell);
  u8F2Hysterese  = eeprom_read_byte(&ee_u8F2Hysterese);
  u8FanSetStatus = eeprom_read_byte(&ee_u8FanSetStatus);
  u8oldF1Swell      = u8F1Swell     ;
  u8oldF1Hysterese  = u8F1Hysterese ;
  u8oldF2Swell      = u8F2Swell     ;
  u8oldF2Hysterese  = u8F2Hysterese ;
  u8oldFanSetStatus = u8FanSetStatus;
}

void writeEEData()
{
  LEDGRUEN_ON;
	MyTimers[TIMER_SAVE_DELAY].state = TM_START; // Speicherverzögerung läuft los
}


