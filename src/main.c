#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

/* Kevin Duong 2025/2026 Elec. Used chatgpt for the most part of the code... Currently, 
the atmospheric pressure is a constant. However, I will aim to replace it with a recorded value */

#define SLEEP_TIME_MS 1000

/* Constant for Pressure-Depth Conversion*/
#define GRAV 9.80665
#define WATER_DENSITY 997.0474 // Fresh Water
#define atmosphericPressure 101500 // In pascals measured at EDC Jan 24 12pm

/* MS5837-30BA (BAR30) commands */
#define CMD_RESET        0x1E
#define CMD_CONVERT_D1   0x48   // Pressure, OSR=4096
#define CMD_CONVERT_D2   0x58   // Temperature, OSR=4096
#define CMD_ADC_READ     0x00

/* DeviceTree node */
#define I2C_NODE DT_NODELABEL(mysensor)

/* Calibration coefficients C1–C6 */
uint16_t C[7];   // index 1..6 used

int main(void)
{
    static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready\n");
        return -1;
    }

    printk("BAR30 init OK\n");

    /* ---- Reset sensor ---- */
    uint8_t cmd = CMD_RESET;
    if (i2c_write_dt(&dev_i2c, &cmd, 1) != 0) {
        printk("Reset failed\n");
        return -1;
    }

    /* IMPORTANT: BAR30 needs longer startup time */
    k_msleep(50);

    /* ---- Read PROM calibration (C1–C6 only) ---- */
    for (int i = 1; i <= 6; i++) {
        uint8_t prom_cmd = 0xA0 + (i * 2);
        uint8_t buf[2];

        if (i2c_write_read_dt(&dev_i2c, &prom_cmd, 1, buf, 2) != 0) {
            printk("PROM read failed at C%d\n", i);
            return -1;
        }

        C[i] = (buf[0] << 8) | buf[1];
    }

    printk("PROM read OK\n");

    while (1) {
        uint8_t buf[3];

        /* ---- Pressure conversion (D1) ---- */
        cmd = CMD_CONVERT_D1;
        i2c_write_dt(&dev_i2c, &cmd, 1);
        k_msleep(20);

        cmd = CMD_ADC_READ;
        i2c_write_read_dt(&dev_i2c, &cmd, 1, buf, 3);

        uint32_t D1 =
            ((uint32_t)buf[0] << 16) |
            ((uint32_t)buf[1] << 8)  |
            buf[2];

        /* ---- Temperature conversion (D2) ---- */
        cmd = CMD_CONVERT_D2;
        i2c_write_dt(&dev_i2c, &cmd, 1);
        k_msleep(20);

        cmd = CMD_ADC_READ;
        i2c_write_read_dt(&dev_i2c, &cmd, 1, buf, 3);

        uint32_t D2 =
            ((uint32_t)buf[0] << 16) |
            ((uint32_t)buf[1] << 8)  |
            buf[2];

        /* ---- Compensation math (datasheet) ---- */
        int32_t dT = D2 - ((int32_t)C[5] << 8);
        int64_t OFF  = ((int64_t)C[2] << 16) + ((int64_t)dT * C[4]) / 128;
        int64_t SENS = ((int64_t)C[1] << 15) + ((int64_t)dT * C[3]) / 256;


        /* ---- Calculations of Pressure and Depth ---- */

        int32_t pressure_pa = (((int64_t)D1 * SENS) / 2097152 - OFF) / 16384;

        double depth = ( pressure_pa - atmosphericPressure ) / ( GRAV * WATER_DENSITY );

        printk("Pressure: %d Pa\n", pressure_pa);
        printk("Depth: %.2lf m\n", depth);

        k_msleep(SLEEP_TIME_MS);
    }
}
