This branch contains the code for saving data to an SD card. Since it needs data to store, it also has the interface for the temperature sensor.

### Status
Developing

### Specs
Uses SPI protocol. The reader is [MEM2052-00-195-00-A](https://gct.co/files/specs/mem2052-spec.pdf).

### Resources
[Nordic Semiconductor Example](https://docs.zephyrproject.org/latest/samples/subsys/fs/fs_sample/README.html)
[Sample Application](https://github.com/zephyrproject-rtos/zephyr/tree/main/samples/subsys/fs/fs_sample)
[Disk Access Documentation](https://docs.zephyrproject.org/latest/services/storage/disk/access.html)
[Log file format specification](https://docs.google.com/document/d/10IAxWH3dzn_0fOTQJNibCKDUpabhp-pmox66Js6HYPk/edit?tab=t.0)