#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

#define NUM_TAPS 31
#define BIT_SHIFT 10

static const int32_t coef[NUM_TAPS] = {(int32_t)(-0.001733f * 1024.0f),
    (int32_t)(-0.001987f * 1024.0f),
    (int32_t)(-0.002422f * 1024.0f),
    (int32_t)(-0.002528f * 1024.0f),
    (int32_t)(-0.001485f * 1024.0f),
    (int32_t)( 0.001858f * 1024.0f),
    (int32_t)( 0.008452f * 1024.0f),
    (int32_t)( 0.018835f * 1024.0f),
    (int32_t)( 0.033008f * 1024.0f),
    (int32_t)( 0.050364f * 1024.0f),
    (int32_t)( 0.069705f * 1024.0f),
    (int32_t)( 0.089383f * 1024.0f),
    (int32_t)( 0.107540f * 1024.0f),
    (int32_t)( 0.122385f * 1024.0f),
    (int32_t)( 0.132458f * 1024.0f),
    (int32_t)( 0.136013f * 1024.0f),
    (int32_t)( 0.132458f * 1024.0f),
    (int32_t)( 0.122385f * 1024.0f),
    (int32_t)( 0.107540f * 1024.0f),
    (int32_t)( 0.089383f * 1024.0f),
    (int32_t)( 0.069705f * 1024.0f),
    (int32_t)( 0.050364f * 1024.0f),
    (int32_t)( 0.033008f * 1024.0f),
    (int32_t)( 0.018835f * 1024.0f),
    (int32_t)( 0.008452f * 1024.0f),
    (int32_t)( 0.001858f * 1024.0f),
    (int32_t)(-0.001485f * 1024.0f),
    (int32_t)(-0.002528f * 1024.0f),
    (int32_t)(-0.002422f * 1024.0f),
    (int32_t)(-0.001987f * 1024.0f),
    (int32_t)(-0.001733f * 1024.0f)};

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