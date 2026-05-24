#include "stm32u5xx_hal.h"

#include "i2c_bus.h"

static I2C_HandleTypeDef hi2c1;

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance != I2C1) return;
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    gpio.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode      = GPIO_MODE_AF_OD;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance != I2C1) return;
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
}

i2c_status_t i2c_bus_init(void) {
    // Timing for 100 kHz with 4 MHz MSI (RCC_MSIRANGE_4).
    // Recalculate with STM32CubeMX if the kernel clock changes.
    hi2c1.Instance              = I2C1;
    hi2c1.Init.Timing           = 0x00303D5Bu;
    hi2c1.Init.OwnAddress1      = 0;
    hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
    hi2c1.Init.OwnAddress2      = 0;
    hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
    hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;
    return HAL_I2C_Init(&hi2c1) == HAL_OK ? I2C_OK : I2C_ERR;
}

i2c_status_t i2c_write(uint8_t dev_addr, const uint8_t *buf, uint16_t len) {
    return HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(dev_addr << 1),
                                   (uint8_t *)buf, len, 100) == HAL_OK
        ? I2C_OK : I2C_ERR;
}

i2c_status_t i2c_read(uint8_t dev_addr, uint8_t *buf, uint16_t len) {
    return HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(dev_addr << 1),
                                  buf, len, 100) == HAL_OK
        ? I2C_OK : I2C_ERR;
}

void i2c_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
