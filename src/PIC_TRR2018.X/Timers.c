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

    TMR0H   =0x7A;
    TMR0L   =0x12;
    TMR0IF=0;
    
    T0CONbits.TMR0ON=1;  // Ré-sactive le timer   
}


/*
 Val_Timer = TMR0L;
        val16 = TMR0H;
        val16 = val16 << 8;
        Val_Timer += val16;
        
        if(Val_Timer == 31000)
        //if(TMR0IF==1)
        {
            arretUrgence=1;
            LATCbits.LATC5 = 1; // a enlever
            TMR0H=BASE_500MSH; // a enlever
            TMR0L=BASE_500MSL; // a enlever
        }
 */