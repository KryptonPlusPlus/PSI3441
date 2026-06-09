#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

// --- log ---
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);
// ---

volatile uint8_t button_interrupt = 0;

// --- accel --
#define ACC_NODE DT_NODELABEL(mma8451q)

#if DT_NODE_HAS_STATUS(ACC_NODE, okay)
    static const struct device *const accel_dev = DEVICE_DT_GET(ACC_NODE);
#else
    #error "Unsupported board."
#endif

struct sensor_value accel_value[3];
void accel_thread_entry(void *arg0, void *arg1, void *arg2)
{
    if (!device_is_ready(accel_dev)) 
    {
        LOG_ERR("acelerometro nao esta pronto");
        return;
    }

    while(1)
    {
        int ret = sensor_sample_fetch(accel_dev);

        if (ret) 
        {
            LOG_ERR("nao foi possivel ler o acelerometro: %d", ret);
            k_sleep(K_MSEC(1000));
            return;
        }

        if(button_interrupt)
        {
            sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_XYZ, accel_value);

            LOG_INF("x: %d.%06d, y: %d.%06d, z: %d.%06d",
                     accel_value[0].val1, abs(accel_value[0].val2),
                     accel_value[1].val1, abs(accel_value[1].val2),
                     accel_value[2].val1, abs(accel_value[2].val2));
        }

        k_sleep(K_MSEC(1000));
    }
}
// ---

// --- adc --
#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL_ID       0
#define ADC_VREF_MV          3300

#define ADC_NODE DT_NODELABEL(adc0)

#if DT_NODE_HAS_STATUS(ADC_NODE, okay)
    static const struct device *const adc_dev = DEVICE_DT_GET(ADC_NODE);
#else
    #error "Unsupported board."
#endif

int16_t sample_buffer;
void adc_thread_entry(void *arg0, void *arg1, void *arg2)
{
    if (!device_is_ready(adc_dev)) 
    {
        LOG_ERR("adc não está pronto.");
        return;
    }

    struct adc_channel_cfg channel_cfg = 
    {
        .gain             = ADC_GAIN,
        .reference        = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id       = ADC_CHANNEL_ID,
        .differential     = 0,
    };

    int ret = adc_channel_setup(adc_dev, &channel_cfg);

    if (ret < 0) 
    {
        LOG_ERR("Erro ao configurar canal ADC: %d", ret);
        return;
    }

    struct adc_sequence sequence = 
    {
        .channels    = BIT(ADC_CHANNEL_ID),
        .buffer      = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution  = ADC_RESOLUTION,
    };

    while(1)
    {
        ret = adc_read(adc_dev, &sequence);

        if (ret) 
        {
            LOG_ERR("Falha na leitura do ADC: %d\n", ret);
        } 
        else 
        {
            int32_t mv = sample_buffer;
            adc_raw_to_millivolts(adc_ref_internal(adc_dev), ADC_GAIN, ADC_RESOLUTION, &mv);

            LOG_INF("adc: %d mV", mv);
        }

        k_sleep(K_MSEC(500));
    }
}
// ---

// --- button ---
#define BUTTON_NODE DT_NODELABEL(user_button_0)

#if DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
    static const struct gpio_dt_spec button_dev = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
#else
    #error "Unsupported board."
#endif

static struct gpio_callback button_callback_data;

void button_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    button_interrupt = !button_interrupt;
}
// ---

// --- thread ---
#define STACK_SIZE 1024

#define ACC_PRIORITY 1
#define ADC_PRIORITY 2

K_THREAD_DEFINE(acc_tid, STACK_SIZE, accel_thread_entry, NULL, NULL, NULL, ACC_PRIORITY, 0, 0);
K_THREAD_DEFINE(adc_tid, STACK_SIZE, adc_thread_entry,   NULL, NULL, NULL, ACC_PRIORITY, 0, 0);
// ---

int main()
{
    gpio_pin_configure_dt(&button_dev, GPIO_INPUT | GPIO_PULL_UP);
    
    gpio_pin_interrupt_configure_dt(&button_dev, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&button_callback_data, button_callback, 1 << button_dev.pin);
    gpio_add_callback(button_dev.port, &button_callback_data);

    while (1) 
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}

