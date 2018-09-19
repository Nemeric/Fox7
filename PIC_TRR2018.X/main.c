/* 
 * File:   main.c
 * Author: rhaynnix
 *
 * Created on 12 septembre 2018, 12:59
 */

#pragma config FOSC = INTIO2    // Oscillator (Internal RC oscillator)
#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled)
#pragma config WDTEN = OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config XINST = OFF       // Extended Instruction Set (Enabled)


#include <xc.h>
#include "pin.h"


//#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Timers.h"



// Clock 64Mhz
// Diviser l'horloge principale par 6666 pour baud uart avec arduino a 9600
// division par 64 -> donne 104 dans le regitre on écrit 103 cf formule datasheet
// BRG16=0 et BRGH=0

#define DELAY_MAX 1000

void initIO()
{
    // Gerer TRIS
}

void initUART()
{
    TRISCbits.TRISC6=0;
    TRISCbits.TRISC7=1;
    
    TXSTA1bits.SYNC = 0; // Async operation
    TXSTA1bits.TX9 = 0;  // No tx of 9th bit
    TXSTA1bits.TXEN = 1;  // Enable transmitter
            
    RCSTA1bits.RX9 = 0;  // No rx of 9th bit
    RCSTA1bits.CREN = 1; // Enable receiver
    RCSTA1bits.SPEN = 1; // Enable serial port
    
    BAUDCON1bits.BRG161=0;
    TXSTA1bits.BRGH1=0;
    SPBRG1=103;
    
    //TXREG1  register data 
    // Trouver le flag
}



void main(int argc, char** argv) 
{
    uint8_t arretUrgence = 0;
    //arretUrgence = 0;
    uint8_t flagUART = 0;
    uint16_t Val_Timer = 0;
    uint16_t Last_Val_Timer = 0;
    uint16_t val16;
  
    // penser à reséparer en fichier propre
    
    Init_Clk();
    initIO();
    initTimers();
    
    //initUART(); ??
    
    // Init le module uart et voir comment récup le flag
    // car on veut juste verif si qqc recu pas besoin lire
    
    // Gestion XBEE-> avec UART (pas besoin de faire plus normalement 
    
    // Gestion ultrason
    
    // active sortie LED
    TRISCbits.TRISC5 = 0;
    
    while(1)
    {
        
        TMR0H   =0x00;
        TMR0L   =0x00;
        Val_Timer = 0;
        while (Val_Timer < 62500) {     //62.5kHz
            Val_Timer = TMR0L;
            val16 = TMR0H;
            val16 = val16 << 8;
            Val_Timer += val16;
            Last_Val_Timer = Val_Timer;
        }
        // joue avec la sortie LED
        LATCbits.LATC5 = 1 - LATCbits.LATC5;
        // Lecture pin in Raspi IO
        
        // test si on a recu un truc avant delay max atteint
        // compile pas car faut remplacer flagUART par le vrai, pas encore trouver ....
        /*
        if(flagUART==1 && TMR0<DELAY_MAX)
        {
            flagUART=0;
            TMR0=0x0;
        }
        
        // si rien recu avant delay max -> arret urgence
        if(TMR0>DELAY_MAX)
        {
            arretUrgence=1;
        }
        
        // Action principale
        if(!arretUrgence)
        {
            // Transmettre info de la raspi (moteur+servo)
            PIN_MOTEUR_OUT=PIN_MOTEUR_IN;
            PIN_SERVO_OUT=PIN_SERVO_IN;
        }*/
        
        
    }

}

