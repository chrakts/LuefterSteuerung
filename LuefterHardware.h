#ifndef LUEFTERHARDWARE_H_INCLUDED
#define LUEFTERHARDWARE_H_INCLUDED

#ifdef AtxMegaSteckdosen_v02
    #define I2C_EXTENDER_ADDRESS 32
    #define RGBLED_ROT_ON     maxTest.clearOutput(5); maxTest.updateGPIO();
    #define RGBLED_ROT_OFF    maxTest.setOutput(5)  ; maxTest.updateGPIO();
    #define RGBLED_GRUEN_ON     maxTest.clearOutput(6); maxTest.updateGPIO();
    #define RGBLED_GRUEN_OFF    maxTest.setOutput(6)  ; maxTest.updateGPIO();
    #define RGBLED_BLAU_ON     maxTest.clearOutput(7); maxTest.updateGPIO();
    #define RGBLED_BLAU_OFF    maxTest.setOutput(7)  ; maxTest.updateGPIO();

    #define RELAIS1_ON     maxTest.clearOutput(0); maxTest.updateGPIO();
    #define RELAIS1_OFF    maxTest.setOutput(0)  ; maxTest.updateGPIO();
    #define RELAIS2_ON     maxTest.clearOutput(1); maxTest.updateGPIO();
    #define RELAIS2_OFF    maxTest.setOutput(1)  ; maxTest.updateGPIO();
    #define RELAIS3_ON     maxTest.clearOutput(2); maxTest.updateGPIO();
    #define RELAIS3_OFF    maxTest.setOutput(2)  ; maxTest.updateGPIO();
#elif BasisAtxMega32_v02
    #define RGBLED_ROT_ON     LEDROT_ON
    #define RGBLED_ROT_OFF    LEDROT_OFF
    #define RGBLED_GRUEN_ON   LEDGRUEN_ON
    #define RGBLED_GRUEN_OFF  LEDGRUEN_OFF
    #define RGBLED_BLAU_ON    LEDBLAU_ON
    #define RGBLED_BLAU_OFF   LEDBLAU_OFF

    #define RELAIS1_PORT    PORTB
    #define RELAIS1_PIN     PIN2_bm
    #define RELAIS2_PORT    PORTB
    #define RELAIS2_PIN     PIN1_bm
    #define RELAIS3_PORT    PORTB
    #define RELAIS3_PIN     PIN0_bm

    #define RELAIS1SETUP    RELAIS1_PORT.DIRSET=RELAIS1_PIN
    #define RELAIS1_ON      RELAIS1_PORT.OUTCLR=RELAIS1_PIN
    #define RELAIS1_OFF     RELAIS1_PORT.OUTSET=RELAIS1_PIN
    #define RELAIS1_TOGGLE  RELAIS1_PORT.OUTTGL=RELAIS1_PIN

    #define RELAIS2SETUP    RELAIS2_PORT.DIRSET=RELAIS2_PIN
    #define RELAIS2_ON      RELAIS2_PORT.OUTCLR=RELAIS2_PIN
    #define RELAIS2_OFF     RELAIS2_PORT.OUTSET=RELAIS2_PIN
    #define RELAIS2_TOGGLE  RELAIS2_PORT.OUTTGL=RELAIS2_PIN

    #define RELAIS3SETUP    RELAIS3_PORT.DIRSET=RELAIS3_PIN
    #define RELAIS3_ON      RELAIS3_PORT.OUTCLR=RELAIS3_PIN
    #define RELAIS3_OFF     RELAIS3_PORT.OUTSET=RELAIS3_PIN
    #define RELAIS3_TOGGLE  RELAIS3_PORT.OUTTGL=RELAIS3_PIN
#else
#endif // BOARD




#endif // LUEFTERHARDWARE_H_INCLUDED
