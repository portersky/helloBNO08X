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

typedef struct {
    float   i, j, k, real;
    uint8_t accuracy;
} bno08x_rotation_vector_t;

bno08x_status_t bno08x_init(void);
bno08x_status_t bno08x_get_product_id(bno08x_product_id_t *out);
bno08x_status_t bno08x_enable_rotation_vector(uint32_t interval_us);
bno08x_status_t bno08x_read_rotation_vector(bno08x_rotation_vector_t *out);
