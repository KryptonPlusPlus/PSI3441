#include "MKL25Z4.h"

// 0 V   -> 0
// 3.3 V -> 2^12 - 1 = 4095
#define ADC_MAX_VALUE 4095U
#define TH_VALUE     (ADC_MAX_VALUE * 80U / 100U)
#define TL_VALUE     (ADC_MAX_VALUE * 20U / 100U)

#define LED_BLUE_PIN  1
#define LED_GREEN_PIN 19
#define ADC_CHANNEL   8  // ptb0

void config_leds();
void config_adc();

uint16_t read_adc(uint8_t channel);

int main()
{
    config_leds();
    config_adc();

    while(1)
    {
        uint16_t value_adc = read_adc(ADC_CHANNEL);

        // TODO: Hysteresis
        // TODO: State machine
        if (value_adc >= TH_VALUE)
        {
            GPIOB->PSOR = (1 << LED_GREEN_PIN); // Apaga LED verde
            GPIOD->PCOR = (1 << LED_BLUE_PIN);  // Liga LED azul
        }
        else if (value_adc <= TL_VALUE)
        {
            GPIOB->PCOR = (1 << LED_GREEN_PIN); // Liga LED verde
            GPIOD->PSOR = (1 << LED_BLUE_PIN);  // Apaga LED azul
        }
        else
        {
            GPIOB->PSOR = (1 << LED_GREEN_PIN); // 
            GPIOD->PSOR = (1 << LED_BLUE_PIN);  // Apaga os dois LEDs
        }
    }

    return 0;
}

void config_leds()
{
    // --- led azul ---
    SIM->SCGC5               |= SIM_SCGC5_PORTD_MASK; // enable clock portd
    PORTD->PCR[LED_BLUE_PIN]  = PORT_PCR_MUX(1);      // configura o mux [10:8] = 001_2
    GPIOD->PDDR              |= 1 << LED_BLUE_PIN;    // define como saida o pino do led azul
    GPIOD->PSOR               = 1 << LED_BLUE_PIN;    // desliga o led   
    // ---

    // --- led verde ---
    SIM->SCGC5                |= SIM_SCGC5_PORTB_MASK; // enable clock portb
    PORTB->PCR[LED_GREEN_PIN]  = PORT_PCR_MUX(1);      // configura o mux [10:8] = 001_2
    GPIOB->PDDR               |= 1 << LED_GREEN_PIN;   // define como saida o pino do led verde
    GPIOB->PSOR                = 1 << LED_GREEN_PIN;   // desliga o led   
    // ---
}

void config_adc()
{
    SIM->SCGC5    |= SIM_SCGC5_PORTB_MASK; // enable clock portb
    PORTB->PCR[0]  = PORT_PCR_MUX(0);      // configura o mux [10:8] = 000_2
    SIM->SCGC6    |= SIM_SCGC6_ADC0_MASK;  // Ativa o clock do adc0
    
    // adc em modo single-ended 12 bits usando bus clock e clock input / 2
    //
    // CFG1:
    //  ADICLK = 0 => Bus clock
    //  MODE   = 1 => 12 bits
    //  ADIV   = 1 => clock / 2
    //
    ADC0->CFG1 |= ADC_CFG1_ADIV(1) | ADC_CFG1_MODE(1) | ADC_CFG1_ADICLK(0);

    // CFG2:
    //  ADLSTS = 3 => 2 extra ADCK cycles
    ADC0->CFG2 = ADC_CFG2_ADLSTS(3);
    ADC0->SC2 = 0;
    ADC0->SC3 = 0;
}

uint16_t read_adc(uint8_t channel)
{
    ADC0->SC1[0] = ADC_SC1_ADCH(channel);

    // TODO: timeout
    while(!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
    
    return ADC0->R[0];
}