#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>

// --- pwm config ---
#define PWM_LED_RED_NODE   DT_ALIAS(pwm_led_red)   // pwm led red
#define PWM_LED_GREEN_NODE DT_ALIAS(pwm_led_green) // pwm led green

#if DT_NODE_HAS_STATUS(PWM_LED_RED_NODE, okay) && DT_NODE_HAS_STATUS(PWM_LED_GREEN_NODE, okay)
    static const struct pwm_dt_spec led_red   = PWM_DT_SPEC_GET(PWM_LED_RED_NODE);
    static const struct pwm_dt_spec led_green = PWM_DT_SPEC_GET(PWM_LED_GREEN_NODE);
#else
    #error "Unsupported board."
#endif
// ---

// --- uart config ---
#define UART_NODE DT_NODELABEL(uart0)

#if DT_NODE_HAS_STATUS(UART_NODE, okay)
    static const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);
#else
    #error "Unsupported board."
#endif

#define BUFFER_SIZE 4 // "100\0"
char receive_bufffer[BUFFER_SIZE];
volatile bool data_received = false;

static void uart_callback(const struct device *dev, void *user_data)
{
    static int buffer_position = 0;

    uint8_t received_byte;

    if (!uart_irq_update(dev)) 
        return;

    while (uart_irq_rx_ready(dev)) 
    {
        if(uart_fifo_read(dev, &received_byte, 1))
        {
            // end receive
            if(received_byte == '\n' || received_byte == '\r')
            {
                if (buffer_position > 0) 
                {
                    receive_bufffer[buffer_position] = '\0';
                    
                    data_received = true;
                    buffer_position = 0;
                }
            }
            else if (buffer_position < (BUFFER_SIZE - 1)) 
            {
                receive_bufffer[buffer_position++] = received_byte;
            }
        }
    }
}
// ---

int main(void)
{
    // --- TODO: ---
    //    A placa frdm-kl25z não roteia o clock do pwm 
    // automaticamente no Zephyr.
    //    Configurar o registrador SIM_SOPT2 (0x40048004) com os 
    // bits (TPMSRC) em "01" (MCGFLLCLK).
    volatile uint32_t *SIM_SOPT2 = (volatile uint32_t *)0x40048004;
    *SIM_SOPT2 |= (1 << 24); 
    // ---

    if (!pwm_is_ready_dt(&led_red) || !pwm_is_ready_dt(&led_green)) 
    {
        printk("Erro: Dispositivo pwm não está pronto!\n");
        return -1;
    }

    if (!device_is_ready(uart_dev)) 
    {
        printk("Erro: Dispositivo uart não está pronto\n"); 
        return -1;
    }

    uart_irq_callback_user_data_set(uart_dev, uart_callback, NULL);
    uart_irq_rx_enable(uart_dev);

    while (1) 
    {
        if(data_received)
        {
            int percentage = atoi(receive_bufffer);

            if (percentage >= 0 && percentage <= 100) 
            {              
                uint32_t duty_red   = (led_red.period   * percentage) / 100;
                uint32_t duty_green = (led_green.period * percentage) / 100;

                if (pwm_set_pulse_dt(&led_red, duty_red) || pwm_set_pulse_dt(&led_green, duty_green / 2)) 
                    printk("Erro ao configurar o pulso do pwm\n");
            } 
            else
            {
                printk("Valor invalido (%d).\n", percentage);
            }

            data_received = false;
        }

        k_msleep(100);
    }

    return 0;
}
