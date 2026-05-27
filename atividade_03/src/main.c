#include "frdm_kl25z.h"

/// delay_ms(int n)
///     n milisegundos
/// Esta função depende do clock default do microcontrolador. 
/// Para o KL25Z a frequência é 21 MHz aproximadamente. 
/// O valor do contador deverá ser ajustado para se conseguir o tempo de espera desejado. 
void delay_ms (int n);

int main()
{
    SIM_SCGC5   |= 1 << 10; // enable clock portb
    PORTB_PCR19  = 1 << 8;  // configura o mux [10:8] = 001_2
    GPIOB_PDDR  |= 1 << 19; // define como saida o pino 19

    while(1)
    {
        GPIOB_PSOR = 1 << 19; // set 
        delay_ms(1000);

        GPIOB_PCOR = 1 << 19; // clear
        delay_ms(1000);
    }

    return 0;
}

void delay_ms (int n) 
{
    int i;
    int j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < 7000; j++)
        {
           __asm volatile ("nop");
        }
    }
}