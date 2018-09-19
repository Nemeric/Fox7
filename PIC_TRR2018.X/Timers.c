#include <xc.h>
#include <stdint.h>
#include "Timers.h"

void Init_Clk (void)
{
    int i = 1;
    OSCCONbits.IRCF = 7;    // choix du 16MHZ
    OSCTUNEbits.PLLEN = 1;  // fait fois 4
    while (i)   // pause de stabilisation de PLL
        i++;
}

void initTimers(void)
{
    // Est ce qu'on garde le 0 pour avoir des ticks seulement toute les 500ms
    // créer une variable qui compte ?
    // Initialisation du timer0
    
    // recoit FOSC/4
    // donc en théorie 64 / 4 => 16MHz
    // div by 256 => 62.5 kHz
    T0CONbits.TMR0ON=0;  // Désactive le timer
    T0CONbits.T08BIT=0;  // Mode 16bit
    T0CONbits.T0CS=0;    // Mode Timer
    T0CONbits.T0SE=0;    // Low to High
    T0CONbits.PSA=0;     // Autorise subdivision
    T0CONbits.T0PS = 7; // Choix subdivision  (div by 256)

    TMR0H   =0x00;
    TMR0L   =0x00;
    
    
    T0CONbits.TMR0ON=1;  // Ré-sactive le timer
    
    //Initialisation timer1? pour ultrason
    
}