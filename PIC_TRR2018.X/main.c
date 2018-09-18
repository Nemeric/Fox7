/* 
 * File:   main.c
 * Author: rhaynnix
 *
 * Created on 12 septembre 2018, 12:59
 */

// peut enlever include pic
#include <pic18f25k80.h>
#include "pin.h"

#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <xc.h>
//#include <libpic30.h>
//#include "qei.h" 

void initTimers()
{
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
}


void main(int argc, char** argv) 
{
    unsigned char arretUrgence;
    arretUrgence=0;
  
    // penser à reséparer en fichier propre
    
    // Initialisation des IO //
    // TRISIO
    // TRISMOT
    // TRISSERVO
    
    initTimers();
    
    // Gestion XBEE //
    // Lire la doc ...
    
    while(1)
    {
        // Lecture pin in Raspi IO
        // Lecture pin in Raspi PWM (moteur, servo)
        // Lecture pin xbee (arret d'urgence)
        // Seulement mettre à 1, ne jamais reset à 0 dans le code -> forcé hard reset -> evite risuqe redémarre
        
        if(!arretUrgence)
        {
            // Transmettre info de la raspi (moteur+servo)
        }
    }

}

