#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <stdint.h>
#include <stdio.h>




#define I2C_NODE DT_NODELABEL(mysensor)
#define DEVICE_ADDRESS 0x29
#define DEVICE_ADDRESS_RW_1 0x51
#define OPR_MODE_REG 0x3D
#define ACC_ONLY_MODE 0x01
#define first_dec 0x293D01
#define ACC_DATA_X_MSB 0x09
#define ACC_DATA_X_LSB 0x08
#define ACC_DATA_Y_MSB 0x0B
#define ACC_DATA_Y_LSB 0x0A
#define ACC_DATA_Z_MSB 0x0D
#define ACC_DATA_Z_LSB 0x0C

int main(void)
{
	static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C device not ready\n");
		return -1;
	} else {
		printk("I2C device ready\n");
	}
	int ret;
	/*Setting up ACC only mode*/
	uint8_t write_buff[3] = {first_dec};
	ret = i2c_write_dt(&dev_i2c, write_buff, sizeof(write_buff));
	if(ret != 0){
		printk("Failed to write first dec");
		return -1;
	}

	uint8_t addr[1]={DEVICE_ADDRESS};
	uint8_t alt_addr[1]={DEVICE_ADDRESS_RW_1};
	uint8_t acc_x_MSB_buff[1]={ACC_DATA_X_MSB};
	uint8_t acc_x_LSB_buff[1]={ACC_DATA_X_LSB};
	uint8_t read_MSB_buff[2];
	uint8_t read_LSB_buff[2];
	while (1) {
		
		/*Write address*/
		ret = i2c_write_dt(&dev_i2c, addr, sizeof(addr));
		if(ret != 0){
		printk("Failed to write address");
		return -1;
		}
		/*Write ACC X MSB register*/
		ret = i2c_write_dt(&dev_i2c, acc_x_MSB_buff, sizeof(acc_x_MSB_buff));
		if(ret != 0){
		printk("Failed to write to ACC X MSB REG");
		return -1;
		}
		/*Put in changed address with RW set to 1*/
		ret = i2c_write_dt(&dev_i2c, alt_addr, sizeof(alt_addr));
		if(ret != 0){
		printk("Failed to write changed address");
		return -1;
		}
		/*Read the MSB ACC value*/
		ret = i2c_read_dt(&dev_i2c, read_MSB_buff, sizeof(read_MSB_buff));
		if(ret != 0){
			printk("Failed to read ACC X MSB");
		}
		/*Write address*/
		ret = i2c_write_dt(&dev_i2c, addr, sizeof(addr));
		if(ret != 0){
		printk("Failed to write address");
		return -1;
		}
		/*Write ACC X LSB register*/
		ret = i2c_write_dt(&dev_i2c, acc_x_LSB_buff, sizeof(acc_x_LSB_buff));
		if(ret != 0){
		printk("Failed to write to ACC X LSB REG");
		return -1;
		}
		/*Put in changed address with RW set to 1*/
		ret = i2c_write_dt(&dev_i2c, alt_addr, sizeof(alt_addr));
		if(ret != 0){
		printk("Failed to write changed address");
		return -1;
		}
		/*Read the LSB ACC value*/
		ret = i2c_read_dt(&dev_i2c, read_LSB_buff, sizeof(read_LSB_buff));
		if(ret != 0){
			printk("Failed to read ACC X LSB");
		}


    // Combine the two arrays into a single 32-bit integer
    uint32_t combined = (read_MSB_buff[0] << 24) | (read_MSB_buff[1] << 16) | (read_LSB_buff[0] << 8) | read_LSB_buff[1];

    // Print the result
    printf("Combined 32-bit value: 0x%08X\n", combined);


	}
}
