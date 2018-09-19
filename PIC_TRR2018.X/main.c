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
#define TIMER_BASE_500MS 0x85CA // Attention avec clk 16MHz, peut pas avec 64MHz

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
    
    //TXREG1  register data // on s'en sert pas
}

void main(int argc, char** argv) 
{

    uint8_t nbTour=0;
    uint8_t tempo=0;
    uint8_t arretUrgence = 0;
    uint16_t Val_Timer = 0;
    uint16_t Last_Val_Timer = 0;
    uint16_t val16;
  
    // penser à reséparer en fichier propre
    
    Init_Clk();
    initIO();
    initTimers();    
    initUART();
    
    // Gestion XBEE-> avec UART (pas besoin de faire plus normalement 
    
    // Gestion ultrason
    // Sur arduino nano on renvoie un pin gpio a 1 tant qu'on detecte l'arche
    // 0 Sinon 
    
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
        // si rien recu avant delay max -> arret urgence
        // flag overflow timer0 activer au bout de 500ms
        if(TMR0IF==1)
        {
            arretUrgence=1;
        }
        // flag uart, test si on a reçu un truc avec l'uart
        if(RC1IF==1)
        {
            RC1IF=0;
            TMR0=TIMER_BASE_500MS;
        }
        
        // Gestion Ultrason pour arret fin
        if(PIN_US)
        {
            tempo+=1;
            if(tempo==5)
                arretUrgence=1;
        }
        else
            tempo=0;
        
        // Action principale
        if(!arretUrgence)
        {
            // Transmettre info de la raspi (moteur+servo)
            PIN_MOTEUR_OUT=PIN_MOTEUR_IN;
            PIN_SERVO_OUT=PIN_SERVO_IN;
        }
        else
        {
            PIN_MOTEUR_OUT=0;
            PIN_SERVO_OUT=0;
        }
        
    }
}

