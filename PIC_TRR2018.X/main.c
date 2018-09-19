/* 
 * File:   main.c
 * Author: rhaynnix
 *
 * Created on 12 septembre 2018, 12:59
 */

// peut enlever include pic et surement d'autre
#include <pic18f25k80.h>
#include "pin.h"

#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <xc.h>
//#include <libpic30.h>
//#include "qei.h" 

//#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled)
#pragma config FOSC = INTIO2    // Oscillator (Internal RC oscillator)
#pragma config PLLCFG = OFF      // PLL x4 Enable bit (Disabled) A voir si on rement en 64MHz (pb timer 500ms)
#pragma config WDTEN = OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)

// Clock 16Mhz
// Diviser l'horloge principale par 6666 pour baud uart avec arduino a 9600
// division par 64 -> donne 104 dans le regitre on écrit 103 cf formule datasheet
// BRG16=0 et BRGH=0

#define DELAY_MAX 1000
#define TIMER_BASE_500MS 0x85CA

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

void initTimers()
{
    // Initialisation du timer0
    T0CONbits.TMR0ON=0;  // Désactive le timer /// ACTIVER ?
    T0CONbits.T08BIT=0;  // Mode 16bit
    T0CONbits.T0CS=0;    // Mode Timer
    T0CONbits.T0SE=0;    // Low to High
    T0CONbits.PSA=1;     // Autorise subdivision
    T0CONbits.T0PS2=1;
    T0CONbits.T0PS1=1;   // Choix subdivision -> 256
    T0CONbits.T0PS0=1;
    TMR0=TIMER_BASE_500MS;   // on setup la valeur de départ du timer pour avoir un overflow à 500 ms
}

void main(int argc, char** argv) 
{
    int nbTour=0;
    int tempo=0;
    
    bool arretUrgence;
    arretUrgence=false;
  
    // penser à reséparer en fichier propre
    
    initIO();
    initTimers();    
    initUART();
    
    // Gestion XBEE-> avec UART (pas besoin de faire plus normalement 
    
    // Gestion ultrason
    // Sur arduino nano on renvoie un pin gpio a 1 tant qu'on detecte l'arche
    // 0 Sinon 
    
    while(1)
    {
        // Lecture pin in Raspi IO
        
        // test si on a recu un truc avant delay max atteint
        // si rien recu avant delay max -> arret urgence
        // flag overflow timer0 activer au bout de 500ms
        if(TMR0IF)
        {
            arretUrgence=true;
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

