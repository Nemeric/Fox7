#include "Timers.h"


void __attribute__((interrupt, auto_psv)) _SPI2Interrupt(void) {
    static uint8_t Start = 0;
    uint8_t Etat_Pin_Laisse = 0;
    static uint8_t Count_Laisse = 0;
    static char count_Evit = 5;

    if (Etat_Pin_Laisse) {
        if (Count_Laisse < 30)
            Count_Laisse++;
    } else {
        if (Count_Laisse)
            Count_Laisse--;
    }
    


    count_Evit--;
    if (!count_Evit) {
//        Gestion_Evitement_Every_few_ms();
        count_Evit = 5;
    }
    
    
//    IFS2bits.SPI2IF = 0;    // clear le flag d'ici
}