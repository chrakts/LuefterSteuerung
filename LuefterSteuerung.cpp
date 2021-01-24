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
  _delay_ms(100);
  /*
  while(!TWI_MasterReady(&twiE_Master))
    ;
  */
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
      double fHumi = fHumidity;
      switch(u8FanActualStatus)
      {
        case FAN_STATUS_OFF:
          if( fHumi > fF1Swell)
            u8FanActualStatus = FAN_STATUS_1;
        break;
        case FAN_STATUS_1:
          if( fHumi < fF1Swell-fF1Hysterese)
            u8FanActualStatus = FAN_STATUS_OFF;
          else if( fHumi > fF2Swell )
            u8FanActualStatus = FAN_STATUS_2;
        break;
        case FAN_STATUS_2:
          if( fHumi < fF2Swell-fF2Hysterese)
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
                cnet.broadcastDouble(fF1Swell,'L','1','L');
                //sprintf(buffer,"%u",(uint8_t)u8F1Swell);
                //cnet.sendStandard(buffer,BROADCAST,'L','1','L','F');
            break;
            case L2SWELLREPORT:
                cnet.broadcastDouble(fF2Swell,'L','1','G');
                //sprintf(buffer,"%u",(uint8_t)u8F2Swell);
                //cnet.sendStandard(buffer,BROADCAST,'L','1','G','F');
            break;
            case L1HYSTREPORT:
                cnet.broadcastDouble(fF1Hysterese,'L','1','H');
                //sprintf(buffer,"%u",(uint8_t)u8F1Hysterese);
                //cnet.sendStandard(buffer,BROADCAST,'L','1','H','F');
            break;
            case L2HYSTREPORT:
                cnet.broadcastDouble(fF2Hysterese,'L','1','I');
                //sprintf(buffer,"%u",(uint8_t)u8F2Hysterese);
                //cnet.sendStandard(buffer,BROADCAST,'L','1','I','F');
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

    if( (foldF1Swell      != fF1Swell     ) |
        (foldF1Hysterese  != fF1Hysterese ) |
        (foldF2Swell      != fF2Swell     ) |
        (foldF2Hysterese  != fF2Hysterese ) |
        (u8oldFanSetStatus != u8FanSetStatus)
      )
    {
      writeEEData();
      foldF1Swell      = fF1Swell     ;
      foldF1Hysterese  = fF1Hysterese ;
      foldF2Swell      = fF2Swell     ;
      foldF2Hysterese  = fF2Hysterese ;
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
  fF1Swell      = eeprom_read_float(&ee_fF1Swell);
  fF1Hysterese  = eeprom_read_float(&ee_fF1Hysterese);
  fF2Swell      = eeprom_read_float(&ee_fF2Swell);
  fF2Hysterese  = eeprom_read_float(&ee_fF2Hysterese);
  u8FanSetStatus = eeprom_read_byte(&ee_u8FanSetStatus);
  foldF1Swell      = fF1Swell     ;
  foldF1Hysterese  = fF1Hysterese ;
  foldF2Swell      = fF2Swell     ;
  foldF2Hysterese  = fF2Hysterese ;
  u8oldFanSetStatus = u8FanSetStatus;
}

void writeEEData()
{
  LEDGRUEN_ON;
	MyTimers[TIMER_SAVE_DELAY].state = TM_START; // Speicherverzögerung läuft los
}


