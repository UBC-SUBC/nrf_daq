#ifndef BAR30_H
#define BAR30_H

#include <zephyr/drivers/i2c.h>
#include <stdint.h>

// Initialize depth sensor
int bar30_init(const struct i2c_dt_spec *i2c);

// Return pressure in pascals
int bar30_read_pressure_pa(const struct i2c_dt_spec *i2c, int32_t *pressure_pa);

#endif