#include "filter.h"

const int32_t fir_coeffs[31] = {
    -0.000648f * (1 << BIT_SHIFT),    -0.001444f * (1 << BIT_SHIFT),    -0.002702f * (1 << BIT_SHIFT),    -0.004441f * (1 << BIT_SHIFT),    -0.006191f * (1 << BIT_SHIFT),
    -0.006959f * (1 << BIT_SHIFT),    -0.005371f * (1 << BIT_SHIFT),    0.000000f * (1 << BIT_SHIFT),    0.010207f * (1 << BIT_SHIFT),    0.025522f * (1 << BIT_SHIFT),
    0.045170f * (1 << BIT_SHIFT),    0.067289f * (1 << BIT_SHIFT),    0.089180f * (1 << BIT_SHIFT),    0.107781f * (1 << BIT_SHIFT),    0.120271f * (1 << BIT_SHIFT),
    0.124672f * (1 << BIT_SHIFT),    0.120271f * (1 << BIT_SHIFT),    0.107781f * (1 << BIT_SHIFT),    0.089180f * (1 << BIT_SHIFT),    0.067289f * (1 << BIT_SHIFT),
    0.045170f * (1 << BIT_SHIFT),    0.025522f * (1 << BIT_SHIFT),    0.010207f * (1 << BIT_SHIFT),    0.000000f * (1 << BIT_SHIFT),    -0.005371f * (1 << BIT_SHIFT),
    -0.006959f * (1 << BIT_SHIFT),    -0.006191f * (1 << BIT_SHIFT),    -0.004441f * (1 << BIT_SHIFT),    -0.002702f * (1 << BIT_SHIFT),    -0.001444f * (1 << BIT_SHIFT),
    -0.000648f * (1 << BIT_SHIFT),
};

void filter_fir_init(struct fir_t *fir)
{
    memset(fir->buffer, 0, sizeof(fir->buffer));
}

struct sensor_value filter_fir(struct fir_t *fir, const int32_t coef[NUM_TAPS], struct sensor_value value)
{
    int32_t val = value.val1 * 1000000 + value.val2;

    for (int i = NUM_TAPS - 1; i > 0; i--)
        fir->buffer[i] = fir->buffer[i - 1];
    
    fir->buffer[0] = val;

    int64_t acc = 0;
    for (int i = 0; i < NUM_TAPS; i++)
        acc += (int64_t)fir->buffer[i] * coef[i];

    acc >>= BIT_SHIFT;
    
    struct sensor_value output;

    output.val1 = acc / 1000000;
    output.val2 = acc % 1000000;

    return output;
}