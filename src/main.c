#include <stdio.h>

#include "stm32u5xx_hal.h"

#include "bno08x.h"
#include "i2c_bus.h"
#include "uart_log.h"

void Error_Handler(void);
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_ICACHE_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    SystemPower_Config();
    MX_ICACHE_Init();
    uart_log_init();

    printf("helloBNO08X starting\r\n");

    if (i2c_bus_init() != I2C_OK) {
        printf("i2c_bus_init failed\r\n");
        Error_Handler();
    }

    // BNO08X needs up to ~100 ms after power-up before the SHTP advertisement
    // is ready to read. Retry with backoff.
    {
        bno08x_status_t s = BNO08X_ERR;
        for (int attempt = 1; attempt <= 10 && s != BNO08X_OK; attempt++) {
            HAL_Delay(100);
            s = bno08x_init();
            if (s != BNO08X_OK)
                printf("bno08x_init attempt %d failed\r\n", attempt);
        }
        if (s != BNO08X_OK) {
            printf("BNO08X not responding: check wiring:\r\n"
                   "  SCL=PB6/D15, SDA=PB7/D14\r\n"
                   "  SA0=GND, PS0=GND, PS1=GND, pull-ups on SCL+SDA\r\n");
            Error_Handler();
        }
    }

    bno08x_product_id_t pid = {0};
    if (bno08x_get_product_id(&pid) == BNO08X_OK) {
        printf("BNO08X ready  sw=%u.%u.%u  reset_cause=%u\r\n",
               pid.sw_major, pid.sw_minor, pid.sw_patch, pid.reset_cause);
    } else {
        printf("BNO08X: product ID read failed\r\n");
    }

    if (bno08x_enable_rotation_vector(100000u) != BNO08X_OK) {
        printf("BNO08X: enable rotation vector failed\r\n");
        Error_Handler();
    }

    printf("entering loop\r\n");
    while (1) {
        HAL_Delay(100);
        bno08x_rotation_vector_t rv;
        if (bno08x_read_rotation_vector(&rv) != BNO08X_OK) {
            printf("rv read fail\r\n");
            continue;
        }
        printf("i=%.4f j=%.4f k=%.4f real=%.4f acc=%u\r\n",
               (double)rv.i, (double)rv.j, (double)rv.k, (double)rv.real,
               (unsigned)rv.accuracy);
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4) != HAL_OK) {
        Error_Handler();
    }

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_4;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                     | RCC_CLOCKTYPE_PCLK3;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

static void SystemPower_Config(void) {
    if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_ICACHE_Init(void) {
    if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_ICACHE_Enable() != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) { }
}

void HAL_MspInit(void) {
    __HAL_RCC_PWR_CLK_ENABLE();
}

void NMI_Handler(void)       { while (1) { } }
void HardFault_Handler(void) { while (1) { } }
void MemManage_Handler(void) { while (1) { } }
void BusFault_Handler(void)  { while (1) { } }
void UsageFault_Handler(void){ while (1) { } }
void SVC_Handler(void)       { }
void DebugMon_Handler(void)  { }
void PendSV_Handler(void)    { }

void SysTick_Handler(void) {
    HAL_IncTick();
}
