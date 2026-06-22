#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

#define NUM_TAPS 31
#define BIT_SHIFT 10

extern const int32_t fir_coeffs[NUM_TAPS];

struct fir_t
{
    int32_t buffer[NUM_TAPS];
};

void filter_fir_init(struct fir_t *buffer);

struct sensor_value filter_fir(struct fir_t *buffer, const int32_t coef[NUM_TAPS], struct sensor_value value);

#endif /* FILTER_H */