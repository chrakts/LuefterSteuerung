#include "myTimers.h"
#include "ledHardware.h"


// 1:  9.9  ms
// 2:  19.8 ms
// 5:  49.4 ms
//10:  99.0 ms
//101: 1000 ms

volatile TIMER MyTimers[MYTIMER_NUM]= {	{TM_START,RESTART_YES,400,0,nextTemperatureStatus},
                                        {TM_START,RESTART_YES,actReportBetweenSensors,0,nextReportStatus},
										{TM_STOP,RESTART_NO,100,0,NULL}		// Timeout-Timer
};



void led1Blinken(uint8_t test)
{
	LED_ROT_TOGGLE;
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
	}
}

void nextReportStatus(uint8_t test)
{
	sendStatusReport = true;
	statusReport+=1;
	if( statusReport > LASTREPORT )
        statusReport = TEMPREPORT;
}



