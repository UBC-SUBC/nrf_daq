#include "bar30.h"

#include <zephyr/kernel.h>

/* MS5837-30BA (BAR30) commands */
#define CMD_RESET        0x1E
#define CMD_CONVERT_D1   0x48   /* Pressure, OSR=4096 */
#define CMD_CONVERT_D2   0x58   /* Temperature, OSR=4096 */
#define CMD_ADC_READ     0x00

/* Calibration coefficients C1–C6 */
static uint16_t C[7];          /* index 1..6 used */
static bool g_inited = false;

/* Helper: read 24-bit ADC value */
static int read_adc_24(const struct i2c_dt_spec *i2c, uint32_t *out)
{
    uint8_t cmd = CMD_ADC_READ;
    uint8_t buf[3];

    if (i2c_write_read_dt(i2c, &cmd, 1, buf, 3) != 0) {
        return -1;
    }

    *out = ((uint32_t)buf[0] << 16) |
           ((uint32_t)buf[1] << 8)  |
           (uint32_t)buf[2];

    return 0;
}

int bar30_init(const struct i2c_dt_spec *i2c)
{
    if (i2c == NULL) {
        return -1;
    }

    /* Reset sensor */
    uint8_t cmd = CMD_RESET;
    if (i2c_write_dt(i2c, &cmd, 1) != 0) {
        return -1;
    }

    /* Startup delay */
    k_msleep(50);

    /* Read PROM calibration C1–C6 */
    for (int i = 1; i <= 6; i++) {
        uint8_t prom_cmd = 0xA0 + (i * 2);
        uint8_t buf[2];

        if (i2c_write_read_dt(i2c, &prom_cmd, 1, buf, 2) != 0) {
            return -1;
        }

        C[i] = ((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
    }

    g_inited = true;
    return 0;
}

int bar30_read_pressure_pa(const struct i2c_dt_spec *i2c, int32_t *pressure_pa)
{
    if (i2c == NULL || pressure_pa == NULL) {
        return -1;
    }

    if (!g_inited) {
        return -1;
    }

    uint32_t D1, D2;
    uint8_t cmd;

    /* ---- Pressure conversion (D1) ---- */
    cmd = CMD_CONVERT_D1;
    if (i2c_write_dt(i2c, &cmd, 1) != 0) {
        return -1;
    }
    k_msleep(20);

    if (read_adc_24(i2c, &D1) != 0) {
        return -1;
    }

    /* ---- Temperature conversion (D2) ---- */
    cmd = CMD_CONVERT_D2;
    if (i2c_write_dt(i2c, &cmd, 1) != 0) {
        return -1;
    }
    k_msleep(20);

    if (read_adc_24(i2c, &D2) != 0) {
        return -1;
    }

    /* ---- Compensation math ---- */
    int32_t dT = (int32_t)D2 - ((int32_t)C[5] << 8);
    int64_t OFF  = ((int64_t)C[2] << 16) + ((int64_t)dT * C[4]) / 128;
    int64_t SENS = ((int64_t)C[1] << 15) + ((int64_t)dT * C[3]) / 256;

    *pressure_pa =
        (int32_t)((((int64_t)D1 * SENS) / 2097152 - OFF) / 16384);

    return 0;
}