#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 500

// Define os LEDs usando Device Tree Alias
#define LED0_NODE DT_ALIAS(led0) // green
#define LED1_NODE DT_ALIAS(led1) // blue
#define LED2_NODE DT_ALIAS(led2) // red

#if DT_NODE_HAS_STATUS(LED0_NODE, okay) && DT_NODE_HAS_STATUS(LED1_NODE, okay) && DT_NODE_HAS_STATUS(LED2_NODE, okay)
    static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
    static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
    static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);    
#else
    #error "Unsupported board: leds device tree alias is not defined"
#endif

// Definição dos estados possíveis para a máquina de estados
//  red
//  yellow = green + red
//  green
typedef enum 
{
    STATE_RED,
    STATE_YELLOW,
    STATE_GREEN
} led_state_t;

void main()
{
    // Verifica se os devices estão prontos
    if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2)) 
    {
        printk("Error: One or more leds devices are not ready\n");
        return;
    }

    // Configura os 3 pinos como saída e inicia deligados
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

    printk("leds ready to blink using a state machine\n");

    // Inicializa a máquina de estados no verde
    led_state_t current_state = STATE_GREEN;

    while (1) 
    {
        // Desliga todos os leds
        gpio_pin_set_dt(&led0, 0);
        gpio_pin_set_dt(&led1, 0);
        gpio_pin_set_dt(&led2, 0);

        switch (current_state) 
        {
            //
            case STATE_GREEN: 
                gpio_pin_set_dt(&led0, 1);
                current_state = STATE_YELLOW;
                break;

            case STATE_YELLOW:
                gpio_pin_set_dt(&led0, 1);
                gpio_pin_set_dt(&led2, 1);
                current_state = STATE_RED;
                break;

            case STATE_RED:
                gpio_pin_set_dt(&led2, 1);
                current_state = STATE_GREEN;
                break;
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return;
}