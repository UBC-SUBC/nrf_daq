 #include <zephyr/kernel.h>
 #include <zephyr/device.h>
 #include <zephyr/devicetree.h>
 #include <zephyr/drivers/i2c.h>
 
 #include <zephyr/sys/printk.h>
 #include <zephyr/sys/byteorder.h>
 
 /* 1000 msec = 1 sec */
 #define SLEEP_TIME_MS 1000
 
 /* STEP 8 - Define the I2C slave device address and the addresses of relevant registers */
 
 
 /* STEP 6 - Get the node identifier of the sensor */
 #define I2C0_NODE DT_NODELABEL(temp_sensor)
 
 int main(void)
 {
 
     int ret;
 
    //  /* STEP 7 - Retrieve the API-specific device structure and make sure that the device is
    //   * ready to use  */
    //  printk("I2C sensor application\n");
    //  static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);
    //  if (!device_is_ready(dev_i2c.bus)) {
    //      printk("I2C device not ready\n");
    //      return -1;
    //  } else {
    //      printk("I2C device ready\n");
    //  }
 
    //  uint8_t i2c_addr[1] = {0x44};
     
    //  ret = i2c_write_dt(&dev_i2c, i2c_addr, 1);
    //  if(ret != 0){
    //      printk("Failed to write to I2C device address %x", dev_i2c.addr);
    //      return -1;
    //  }
 
    //  uint8_t config[2];
    //  sys_put_be16(0x2400, config);
    //  ret = i2c_write_dt(&dev_i2c, config, sizeof(config));
    //  if(ret != 0){
    //      printk("Failed to write to I2C device address %x", dev_i2c.addr);
    //      return -1;
    //  }
    //  while (1) {
    //      /* STEP 10 - Read the temperature from the sensor */
         
    //      uint8_t read_buf[6];
    //      ret = i2c_write_read_dt(&dev_i2c, i2c_addr, 1, read_buf, 6);
    //      if(ret != 0){
    //          printk("Failed to write/read I2C device address %x for temperature or humidity", dev_i2c.addr);
    //      }
 
    //      /* STEP 11 - Convert the two bytes to a 16-bits */
    //      uint16_t temp = (read_buf[0] << 8) | read_buf[1];
    //      uint16_t humidity = (read_buf[3] << 8) | read_buf[4];
 
    //      // Convert to engineering units 
    //      double cTemp = temp * 0.0625;
    //      double fTemp = cTemp * 1.8 + 32;
 
    //      //Print reading to console  
    //      printk("Temperature in Celsius : %.2f C \n", cTemp);
    //      printk("Temperature in Fahrenheit : %.2f F \n", fTemp);
 
    //      printk("Humidity : %d %% \n", humidity);
 
    //      k_msleep(SLEEP_TIME_MS);
    //  }
 }
 