#pragma once
#include <stdint.h>

typedef enum { I2C_OK = 0, I2C_ERR = -1 } i2c_status_t;

i2c_status_t i2c_bus_init(void);
i2c_status_t i2c_write(uint8_t dev_addr, uint8_t const *buf, uint16_t len);
i2c_status_t i2c_read(uint8_t dev_addr, uint8_t *buf, uint16_t len);
void         i2c_delay_ms(uint32_t ms);
