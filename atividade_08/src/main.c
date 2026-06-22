#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/logging/log.h>

#include "filter.h"

// --- log ---
LOG_MODULE_REGISTER(app, LOG_LEVEL_ERR);
// ---

// --- accel ---
#define ACCEL_NODE DT_ALIAS(accel0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(ACCEL_NODE), "Unsupported board.");
static const struct device *const accel_dev = DEVICE_DT_GET(ACCEL_NODE);

struct __attribute__((packed)) data_packet_t
{
    uint8_t header[2];   // Marcador de início: '$' e 'A' (0x24, 0x41)
    uint32_t sequence;   // 4 bytes
    int64_t ts;          // 8 bytes
    int32_t x_val1;      // 4 bytes
    int32_t x_val2;      // 4 bytes
    int32_t y_val1;      // 4 bytes
    int32_t y_val2;      // 4 bytes
    int32_t z_val1;      // 4 bytes
    int32_t z_val2;      // 4 bytes
}; // Total: 2 + 4 + 8 + (6 * 4) = 38 bytes

K_MSGQ_DEFINE(packet, sizeof(struct data_packet_t), 10, 4);
K_TIMER_DEFINE(sample_timer, NULL, NULL);

static struct fir_t filter[3];

void accel_thread_data_acquisition(void *arg0, void *arg1, void *arg2)
{
    filter_fir_init(&filter[0]);
    filter_fir_init(&filter[1]);
    filter_fir_init(&filter[2]);

    if (!device_is_ready(accel_dev)) 
    {
        LOG_ERR("Acelerômetro não está pronto.");
        return;
    }

    struct sensor_value odr_attr;
    odr_attr.val1 = 800; // 400 Hz
    odr_attr.val2 = 0;

    int ret = sensor_attr_set(accel_dev, SENSOR_CHAN_ALL, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
    if (ret < 0)
    {
        LOG_ERR("Erro ao configurar ODR: %d", ret);
        return;
    }

    struct sensor_value data[3];

    k_timer_start(&sample_timer, K_NO_WAIT, K_USEC(1250));
    uint32_t packet_counter = 0;

    while(1)
    {   
        k_timer_status_sync(&sample_timer);

        LOG_DBG("[%s] inicio sensor_sample_fetch", k_thread_name_get(k_current_get()));
        int ret = sensor_sample_fetch(accel_dev);
        LOG_DBG("[%s] fim sensor_sample_fetch", k_thread_name_get(k_current_get()));

        if (ret) 
        {
            LOG_ERR("Não foi possível ler o acelerômetro: %d", ret);
        }
        else
        {
            ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_XYZ, data);
            if (ret) 
                LOG_ERR("Erro lendo canais: %d", ret);

            LOG_INF("\x1b[36mx: %d.%06d, y: %d.%06d, z: %d.%06d\x1b[0m",
                    data[0].val1, abs(data[0].val2),
                    data[1].val1, abs(data[1].val2),
                    data[2].val1, abs(data[2].val2));
            
            data[0] = filter_fir(&filter[0], fir_coeffs, data[0]); //
            data[1] = filter_fir(&filter[1], fir_coeffs, data[1]); //
            data[2] = filter_fir(&filter[2], fir_coeffs, data[2]); // aplica o filtro nos 3 eixos

            struct data_packet_t data_packet = {
                .header = {'$', 'A'},
                .sequence = packet_counter++,
                .ts = k_uptime_get(),
                .x_val1 = data[0].val1,
                .x_val2 = data[0].val2,
                .y_val1 = data[1].val1,
                .y_val2 = data[1].val2,
                .z_val1 = data[2].val1,
                .z_val2 = data[2].val2
            };

            while (k_msgq_put(&packet, &data_packet, K_NO_WAIT) != 0)
            {
                LOG_ERR("purge sensor_value");
                k_msgq_purge(&packet);
                k_msgq_put(&packet, &data_packet, K_NO_WAIT);
            }
        }
    }
}
// ---

// --- uart ---
const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

void accel_thread_communication(void *arg0, void *arg1, void *arg2)
{
    struct data_packet_t single_packet;

    while(1)
    {
        LOG_DBG("[%s] dormiu", k_thread_name_get(k_current_get()));
        k_msgq_get(&packet, &single_packet, K_FOREVER);
        LOG_DBG("[%s] acordou", k_thread_name_get(k_current_get()));

        uint8_t *ptr = (uint8_t *)&single_packet;
        for (int i = 0; i < sizeof(struct data_packet_t); i++)
            uart_poll_out(uart_dev, ptr[i]);
    }
}
// ---

// --- threads ---
#define STACK_SIZE 512

#define ACC_PRIORITY 1

K_THREAD_DEFINE(acc_acquisition_id,   STACK_SIZE, accel_thread_data_acquisition, NULL, NULL, NULL, ACC_PRIORITY,     0, 0);
K_THREAD_DEFINE(acc_communication_id, STACK_SIZE, accel_thread_communication,    NULL, NULL, NULL, ACC_PRIORITY + 1, 0, 0);
// ---

int main()
{
    // --- threads ---
    k_thread_name_set(acc_acquisition_id,   "ACC_ACQ");
    k_thread_name_set(acc_communication_id, "ACC_COM");
    // ---
    
    while (1) 
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}