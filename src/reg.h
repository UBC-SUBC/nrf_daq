
// MLX90393-SLQ-ABA-011-RE 

#define CONFIG_REG      0x10
#define CONFIG_VAL      0x16
#define RM_REG          0x4F
#define RST_REG         0xF0
#define EX_REG          0x80
#define GAIN_REG        0x00
#define READ_CMD        0x50
#define WRITE_CMD       0x60

#define SLEEP_TIME_US   1500

// bool Adafruit_MLX90393::_init(void) {

//     if (!exitMode())
//       return false;
  
//     if (!reset())
//       return false;
  
//     /* Set gain and sensor config. */
//     if (!setGain(MLX90393_GAIN_1X)) {
//       return false;
//     }
  
//     /* Set resolution. */
//     if (!setResolution(MLX90393_X, MLX90393_RES_16))
//       return false;
//     if (!setResolution(MLX90393_Y, MLX90393_RES_16))
//       return false;
//     if (!setResolution(MLX90393_Z, MLX90393_RES_16))
//       return false;
  
//     /* Set oversampling. */
//     if (!setOversampling(MLX90393_OSR_3))
//       return false;
  
//     /* Set digital filtering. */
//     if (!setFilter(MLX90393_FILTER_7))
//       return false;
  
//     /* set INT pin to output interrupt */
//     if (!setTrigInt(false)) {
//       return false;
//     }
  
//     return true;
//   }