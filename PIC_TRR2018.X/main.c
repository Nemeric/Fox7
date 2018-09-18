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

#pragma config FOSC = INTIO2    // Oscillator (Internal RC oscillator)
#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled)
#pragma config WDTEN = OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)

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

void initTimers()
{
    // Est ce qu'on garde le 0 pour avoir des ticks seulement toute les 500ms
    // créer une variable qui compte ?
    // Initialisation du timer0
    T0CONbits.TMR0ON=0;  // Désactive le timer
    T0CONbits.T08BIT=0;  // Mode 16bit
    T0CONbits.T0CS=0;    // Mode Timer
    T0CONbits.T0SE=0;    // Low to High
    T0CONbits.PSA=0;     // Autorise subdivision
    T0CONbits.T0PS2=0;
    T0CONbits.T0PS1=0;   // Choix subdivision
    T0CONbits.T0PS0=0;
    TMR0=0x00;
    
    //Initialisation timer1? pour ultrason
    
}


void main(int argc, char** argv) 
{
    bool arretUrgence;
    arretUrgence=false;
  
    // penser à reséparer en fichier propre
    
    initIO();
    initTimers();
    
    //initUART(); ??
    
    // Init le module uart et voir comment récup le flag
    // car on veut juste verif si qqc recu pas besoin lire
    
    // Gestion XBEE-> avec UART (pas besoin de faire plus normalement 
    
    // Gestion ultrason

    
    while(1)
    {
        // Lecture pin in Raspi IO
        
        // test si on a recu un truc avant delay max atteint
        // compile pas car faut remplacer flagUART par le vrai, pas encore trouver ....
        if(flagUART==1 && TMR0<DELAY_MAX)
        {
            flagUART=0;
            TMR0=0x0;
        }
        
        // si rien recu avant delay max -> arret urgence
        if(TMR0>DELAY_MAX)
        {
            arretUrgence=true;
        }
        
        // Action principale
        if(!arretUrgence)
        {
            // Transmettre info de la raspi (moteur+servo)
            PIN_MOTEUR_OUT=PIN_MOTEUR_IN;
            PIN_SERVO_OUT=PIN_SERVO_IN;
        }
    }

}

