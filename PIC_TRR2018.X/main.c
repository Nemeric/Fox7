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
#define BASE_500MSH 0x85
#define BASE_500MSL 0xCA

void initIO()
{
    // Gerer TRIS
    
    // active sortie LED
    TRISCbits.TRISC5 = 0;
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
}

void main(int argc, char** argv) 
{
    //uint8_t nbTour=0;
    //uint8_t tempo=0;
    uint8_t arretUrgence = 0;
  
    // penser à reséparer en fichier propre
    
    Init_Clk();
    initIO();
    initTimers();    
    initUART();
    
    LATCbits.LATC5 = 0; // a enlever
    
    // Gestion XBEE-> avec UART (pas besoin de faire plus normalement 
    
    // Gestion ultrason
    
    while(1)
    {        
        // Lecture pin in Raspi IO
        
        // test si on a recu un truc avant delay max atteint
        // si rien recu avant delay max -> arret urgence
        // flag overflow timer0 activer au bout de 500ms   
        if(TMR0IF)
        {
            arretUrgence=1;
            LATCbits.LATC5 = 0;
        } 
         
        // flag uart, test si on a reçu un truc avec l'uart
        if(RC1IF)
        //if(RCREG1==0x46)
        {
            RC1IF=0; // Baisse le flag
            RCREG1=0x00;
            TMR0H=BASE_500MSH; // Reset timer
            TMR0L=BASE_500MSL;
            TMR0IF=0;
            arretUrgence=0;
            LATCbits.LATC5 = 1; // allume led
        }
        
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

