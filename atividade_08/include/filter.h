#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

#define NUM_TAPS 31
#define BIT_SHIFT 10

static const int32_t fir_coeffs[31] = {
    -0.000648f * (1 << BIT_SHIFT),    -0.001444f * (1 << BIT_SHIFT),    -0.002702f * (1 << BIT_SHIFT),    -0.004441f * (1 << BIT_SHIFT),    -0.006191f * (1 << BIT_SHIFT),
    -0.006959f * (1 << BIT_SHIFT),    -0.005371f * (1 << BIT_SHIFT),    0.000000f * (1 << BIT_SHIFT),    0.010207f * (1 << BIT_SHIFT),    0.025522f * (1 << BIT_SHIFT),
    0.045170f * (1 << BIT_SHIFT),    0.067289f * (1 << BIT_SHIFT),    0.089180f * (1 << BIT_SHIFT),    0.107781f * (1 << BIT_SHIFT),    0.120271f * (1 << BIT_SHIFT),
    0.124672f * (1 << BIT_SHIFT),    0.120271f * (1 << BIT_SHIFT),    0.107781f * (1 << BIT_SHIFT),    0.089180f * (1 << BIT_SHIFT),    0.067289f * (1 << BIT_SHIFT),
    0.045170f * (1 << BIT_SHIFT),    0.025522f * (1 << BIT_SHIFT),    0.010207f * (1 << BIT_SHIFT),    0.000000f * (1 << BIT_SHIFT),    -0.005371f * (1 << BIT_SHIFT),
    -0.006959f * (1 << BIT_SHIFT),    -0.006191f * (1 << BIT_SHIFT),    -0.004441f * (1 << BIT_SHIFT),    -0.002702f * (1 << BIT_SHIFT),    -0.001444f * (1 << BIT_SHIFT),
    -0.000648f * (1 << BIT_SHIFT),
};

static int32_t buffer[NUM_TAPS];

static struct sensor_value filter_fir(const int32_t coef[NUM_TAPS], struct sensor_value value)
{
    int32_t val = value.val1 * 1000000 + value.val2;

    for (int i = NUM_TAPS - 1; i > 0; i--)
        buffer[i] = buffer[i - 1];
    
    buffer[0] = val;

    int64_t acc = 0;
    for (int i = 0; i < NUM_TAPS; i++)
        acc += (int64_t)buffer[i] * coef[i];

    acc >>= BIT_SHIFT;
    
    struct sensor_value output;

    output.val1 = acc / 1000000;
    output.val2 = acc % 1000000;

    return output;
}

#endif /* FILTER_H */