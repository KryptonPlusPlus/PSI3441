// TODO: usar uma maquina de estados para controlar cada estado do sistema
// TODO: separar os "drivers" em arquivos separados
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>

// --- log ---
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);
// ---

// --- threads ---
K_SEM_DEFINE(acc_enable_sem, 0, 1);
atomic_t accel_enable = ATOMIC_INIT(0);
// ---

// --- accel ---
#define ACCEL_NODE DT_ALIAS(accel0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(ACCEL_NODE), "Unsupported board.");
static const struct device *const accel_dev = DEVICE_DT_GET(ACCEL_NODE);

struct sensor_value accel_value[3];
void accel_thread_entry(void *arg0, void *arg1, void *arg2)
{
    if (!device_is_ready(accel_dev)) 
    {
        LOG_ERR("Acelerômetro não está pronto.");
        return;
    }

    while(1)
    {
        // TODO: suspender o sensor enquanto ele está desabilitado
        // TODO: Utilizar o trigger do sensor
        //      &mmaa8451 {
        //       int1-gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
        //      };
        //      sensor_trigger_set(...);

        if (!atomic_get(&accel_enable))
        {
            LOG_DBG("[%s] desativado", k_thread_name_get(k_current_get()));
            k_sem_take(&acc_enable_sem, K_FOREVER);
        }
        
        LOG_DBG("[%s] inicio sensor_sample_fetch", k_thread_name_get(k_current_get()));
        int ret = sensor_sample_fetch(accel_dev);
        LOG_DBG("[%s] fim sensor_sample_fetch", k_thread_name_get(k_current_get()));

        // for(int i = 0; i < 1000000; i++)
        //     __asm volatile ("nop");

        if (ret) 
        {
            LOG_ERR("Não foi possível ler o acelerômetro: %d", ret);
        }
        else
        {
            ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_XYZ, accel_value);
            if (ret) 
                LOG_ERR("Erro lendo canais: %d", ret);

            LOG_INF("\x1b[36mx: %d.%06d, y: %d.%06d, z: %d.%06d\x1b[0m",
                    accel_value[0].val1, abs(accel_value[0].val2),
                    accel_value[1].val1, abs(accel_value[1].val2),
                    accel_value[2].val1, abs(accel_value[2].val2));
        }

        LOG_DBG("[%s] dormiu", k_thread_name_get(k_current_get()));
        k_sleep(K_MSEC(1000));
        LOG_DBG("[%s] acordou", k_thread_name_get(k_current_get()));
    }
}
// ---

// --- adc --
#define ADC_NODE DT_PATH(zephyr_user)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(ADC_NODE), "Unsupported board.");
static const struct adc_dt_spec adc_spec = ADC_DT_SPEC_GET(ADC_NODE);

int16_t sample_buffer;
void adc_thread_entry(void *arg0, void *arg1, void *arg2)
{
    if (!adc_is_ready_dt(&adc_spec)) 
    {
        LOG_ERR("ADC não está pronto.");
        return;
    }

    int ret = adc_channel_setup_dt(&adc_spec);

    if (ret < 0)
    {
        LOG_ERR("Erro ao configurar o ADC: %d", ret);
        return;
    }

    struct adc_sequence sequence = 
    {
        .buffer      = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
    };

    adc_sequence_init_dt(&adc_spec, &sequence);

    while(1)
    {
        ret = adc_read(adc_spec.dev, &sequence);

        if (ret) 
        {
            LOG_ERR("Falha na leitura do ADC: %d", ret);
        } 
        else 
        {
            int32_t mv = sample_buffer;
            ret = adc_raw_to_millivolts_dt(&adc_spec, &mv);
            if (ret == 0)
                LOG_INF("\x1b[34madc: %d mV\x1b[0m", mv);
        }
        LOG_DBG("[%s] dormiu", k_thread_name_get(k_current_get()));
        k_sleep(K_MSEC(500));
        LOG_DBG("[%s] acordou", k_thread_name_get(k_current_get()));
    }
}
// ---

// --- button ---
#define BUTTON_NODE DT_ALIAS(sw0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(BUTTON_NODE), "Unsupported board.");
static const struct gpio_dt_spec button_spec = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback button_callback_data;

void button_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // TODO: adicionar verificação de debounce
    if(atomic_cas(&accel_enable, 0, 1))
        k_sem_give(&acc_enable_sem);
    else
        atomic_set(&accel_enable, 0);
}
// ---

// --- threads ---
#define STACK_SIZE 512

#define ACC_PRIORITY 1
#define ADC_PRIORITY 1

K_THREAD_DEFINE(acc_tid, STACK_SIZE, accel_thread_entry, NULL, NULL, NULL, ACC_PRIORITY, 0, 0);
K_THREAD_DEFINE(adc_tid, STACK_SIZE, adc_thread_entry,   NULL, NULL, NULL, ADC_PRIORITY, 0, 0);
// ---

int main()
{
    // --- threads ---
    k_thread_name_set(acc_tid, "ACC");
    k_thread_name_set(adc_tid, "ADC");
    // ---
    int ret = 0;

    if (!gpio_is_ready_dt(&button_spec))
    {
        LOG_ERR("Botão não está pronto");
        return -1;
    }
    ret = gpio_pin_configure_dt(&button_spec, GPIO_INPUT);
    if (ret < 0) 
    {
        LOG_ERR("Falha na configuração do gpio: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button_spec, GPIO_INT_EDGE_FALLING);
    if (ret < 0) 
    {
        LOG_ERR("Falha na configuração da interrupção: %d", ret);
        return ret;
    }

    gpio_init_callback(&button_callback_data, button_callback, 1 << button_spec.pin);
    ret = gpio_add_callback(button_spec.port, &button_callback_data);
    if (ret < 0) 
    {
        LOG_ERR("Falha na configuração da função de callback: %d", ret);
        return ret;
    }

    while (1) 
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}

