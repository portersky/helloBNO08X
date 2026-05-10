#include "unity.h"
#include "src/bno08x.h"
#include "Mocki2c_bus.h"

void setUp(void)    { Mocki2c_bus_Init(); i2c_delay_ms_Ignore(); }
void tearDown(void) { Mocki2c_bus_Verify(); Mocki2c_bus_Destroy(); }

// ---- bno08x_init ----

void test_init_ok_when_sensor_responds(void) {
    uint8_t hdr[4] = { 10, 0, 0, 0 }; // pkt_len = 10
    // Peek: read 4-byte header
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    // Consume: read full pkt_len bytes
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 10, I2C_OK);
    i2c_read_IgnoreArg_buf();
    TEST_ASSERT_EQUAL(BNO08X_OK, bno08x_init());
}

void test_init_err_when_i2c_fails(void) {
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_ERR);
    i2c_read_IgnoreArg_buf();
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_init());
}

void test_init_err_when_zero_length_header(void) {
    uint8_t hdr[4] = { 0, 0, 0, 0 };
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    // No second read — pkt_len == 0 causes early return
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_init());
}

void test_init_err_when_drain_fails(void) {
    uint8_t hdr[4] = { 10, 0, 0, 0 };
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 10, I2C_ERR);
    i2c_read_IgnoreArg_buf();
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_init());
}

// ---- bno08x_get_product_id ----

void test_get_product_id_parses_response(void) {
    uint8_t rsp[20] = {0};
    rsp[4]  = 0xF8u; // REP_PRODUCT_ID_RSP
    rsp[5]  = 2;     // reset_cause
    rsp[6]  = 4;     // sw_major
    rsp[7]  = 3;     // sw_minor
    rsp[16] = 0x07u; // sw_patch LSB
    rsp[17] = 0x00u; // sw_patch MSB

    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_OK);
    i2c_write_IgnoreArg_buf();
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 20, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(rsp, 20);

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_OK, bno08x_get_product_id(&id));
    TEST_ASSERT_EQUAL_UINT8(2, id.reset_cause);
    TEST_ASSERT_EQUAL_UINT8(4, id.sw_major);
    TEST_ASSERT_EQUAL_UINT8(3, id.sw_minor);
    TEST_ASSERT_EQUAL_UINT16(7, id.sw_patch);
}

void test_get_product_id_err_on_write_fail(void) {
    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_ERR);
    i2c_write_IgnoreArg_buf();

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_get_product_id(&id));
}

void test_get_product_id_err_on_read_fail(void) {
    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_OK);
    i2c_write_IgnoreArg_buf();
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 20, I2C_ERR);
    i2c_read_IgnoreArg_buf();

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_get_product_id(&id));
}

void test_get_product_id_err_on_wrong_report_id(void) {
    uint8_t rsp[20] = {0};
    rsp[4] = 0x01u; // not REP_PRODUCT_ID_RSP

    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_OK);
    i2c_write_IgnoreArg_buf();
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 20, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(rsp, 20);

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_get_product_id(&id));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_ok_when_sensor_responds);
    RUN_TEST(test_init_err_when_i2c_fails);
    RUN_TEST(test_init_err_when_zero_length_header);
    RUN_TEST(test_init_err_when_drain_fails);
    RUN_TEST(test_get_product_id_parses_response);
    RUN_TEST(test_get_product_id_err_on_write_fail);
    RUN_TEST(test_get_product_id_err_on_read_fail);
    RUN_TEST(test_get_product_id_err_on_wrong_report_id);
    return UNITY_END();
}
