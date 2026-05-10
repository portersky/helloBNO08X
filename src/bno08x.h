#pragma once
#include <stdint.h>

#define BNO08X_ADDR  0x4Au

typedef enum { BNO08X_OK = 0, BNO08X_ERR } bno08x_status_t;

typedef struct {
    uint8_t  reset_cause;
    uint8_t  sw_major;
    uint8_t  sw_minor;
    uint16_t sw_patch;
} bno08x_product_id_t;

bno08x_status_t bno08x_init(void);
bno08x_status_t bno08x_get_product_id(bno08x_product_id_t *out);
