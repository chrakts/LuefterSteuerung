#include "myTimers.h"
#include "ledHardware.h"


// 1:  9.9  ms
// 2:  19.8 ms
// 5:  49.4 ms
//10:  99.0 ms
//101: 1000 ms

volatile TIMER MyTimers[MYTIMER_NUM]= {	{TM_START,RESTART_YES,400,0,nextTemperatureStatus},
                                        {TM_START,RESTART_YES,actReportBetweenSensors,0,nextReportStatus},
                                        {TM_STOP,RESTART_NO,5000,0,nowSaveEEProm}
};



void led1Blinken(uint8_t test)
{
	LEDROT_TOGGLE;
}

void led2Blinken(uint8_t test)
{
//	LED_KLINGEL_TOGGLE;
}


// NOTHING_CLIMA_TODO ->  START_CONVERSION -> WAIT_CONVERSION -> GET_TEMPERATURE -> NOTHING_CLIMA_TODO
void nextTemperatureStatus(uint8_t test)
{
	switch (statusSensoren)
	{
		case KLIMASENSOR:
			switch(statusKlima)
			{
				case WAIT_WAKEUP:
					statusKlima = START_CONVERSION;
				break;
				case WAIT_CONVERSION:
					statusKlima = READ_CONVERSION;
				break;
				case WAIT_READ:
					statusKlima = CALC_CONVERSION0;
				break;
			}
		break;
    case LASTSENSOR:
      switch(statusLastSensor)
      {
        case WAIT_LAST:
          statusLastSensor=READY_LAST;
        break;
      }
    break;
	}

}

void nowSaveEEProm(uint8_t test)
{
  eeprom_update_float(&ee_fF1Swell,fF1Swell);
  eeprom_update_float(&ee_fF1Hysterese,fF1Hysterese);
  eeprom_update_float(&ee_fF2Swell,fF2Swell);
  eeprom_update_float(&ee_fF2Hysterese,fF2Hysterese);
  eeprom_update_byte(&ee_u8FanSetStatus,u8FanSetStatus);
  LEDGRUEN_OFF;
}


void nextReportStatus(uint8_t test)
{
	sendStatusReport = true;
	statusReport+=1;
	if( statusReport > LASTREPORT )
        statusReport = TEMPREPORT;
}



