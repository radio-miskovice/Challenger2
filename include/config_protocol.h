#ifdef CONFIG_BAUDRATE_OVERRIDE
const unsigned long SERIAL_SPEED = CONFIG_BAUDRATE_OVERRIDE;
#else
const unsigned long SERIAL_SPEED = 1200;
#endif