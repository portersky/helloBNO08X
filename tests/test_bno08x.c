#include "unity.h"
#include "src/bno08x.h"
#include "Mocki2c_bus.h"

void setUp(void)    { Mocki2c_bus_Init(); i2c_delay_ms_Ignore(); }
void tearDown(void) { Mocki2c_bus_Verify(); Mocki2c_bus_Destroy(); }

// ---- bno08x_init ----

void test_init_ok_when_sensor_responds(void) {
    uint8_t hdr[4]   = { 10, 0, 0, 0 }; // pkt_len = 10
    uint8_t empty[4] = {  0, 0, 0, 0 }; // pkt_len = 0 — buffer drained
    // Peek: non-empty packet
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    // Consume
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 10, I2C_OK);
    i2c_read_IgnoreArg_buf();
    // Peek: empty — signals buffer drained
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(empty, 4);
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

void test_init_ok_when_second_peek_nacks(void) {
    uint8_t hdr[4] = { 10, 0, 0, 0 };
    // Peek: non-empty packet
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    // Consume
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 10, I2C_OK);
    i2c_read_IgnoreArg_buf();
    // Second peek NACKs — buffer empty, should still succeed
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_ERR);
    i2c_read_IgnoreArg_buf();
    TEST_ASSERT_EQUAL(BNO08X_OK, bno08x_init());
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
    uint8_t hdr[4] = { 20, 0, 0, 0 }; // pkt_len = 20
    uint8_t pkt[20] = {0};
    pkt[4]  = 0xF8u; // REP_PRODUCT_ID_RSP
    pkt[5]  = 2;     // reset_cause
    pkt[6]  = 4;     // sw_major
    pkt[7]  = 3;     // sw_minor
    pkt[16] = 0x07u; // sw_patch LSB
    pkt[17] = 0x00u; // sw_patch MSB

    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_OK);
    i2c_write_IgnoreArg_buf();
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 20, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(pkt, 20);

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
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_ERR);
    i2c_read_IgnoreArg_buf();

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_get_product_id(&id));
}

void test_get_product_id_err_on_packet_read_fail(void) {
    uint8_t hdr[4] = { 20, 0, 0, 0 };

    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_OK);
    i2c_write_IgnoreArg_buf();
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 20, I2C_ERR);
    i2c_read_IgnoreArg_buf();

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_get_product_id(&id));
}

void test_get_product_id_err_on_wrong_report_id(void) {
    uint8_t hdr[4] = { 20, 0, 0, 0 };
    uint8_t pkt[20] = {0};
    pkt[4] = 0x01u; // not REP_PRODUCT_ID_RSP

    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 6, I2C_OK);
    i2c_write_IgnoreArg_buf();
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 20, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(pkt, 20);

    bno08x_product_id_t id = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_get_product_id(&id));
}

// ---- bno08x_enable_rotation_vector ----

void test_enable_rotation_vector_ok(void) {
    // 4-byte SHTP header + 17-byte Set Feature Command payload = 21
    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 21, I2C_OK);
    i2c_write_IgnoreArg_buf();
    TEST_ASSERT_EQUAL(BNO08X_OK, bno08x_enable_rotation_vector(100000u));
}

void test_enable_rotation_vector_err_on_write_fail(void) {
    i2c_write_ExpectAndReturn(BNO08X_ADDR, NULL, 21, I2C_ERR);
    i2c_write_IgnoreArg_buf();
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_enable_rotation_vector(100000u));
}

// ---- bno08x_read_rotation_vector ----

void test_read_rotation_vector_parses_report(void) {
    uint8_t hdr[4] = { 18, 0, 0, 0 };
    uint8_t pkt[18] = {0};
    pkt[0] = 18; pkt[2] = 3; pkt[4] = 0x05u; // channel=3, report=RV
    pkt[6] = 2;                 // accuracy = 2
    pkt[8]  = 0x00; pkt[9]  = 0x10; // i = 0x1000 = 4096 → 0.25
    pkt[14] = 0x00; pkt[15] = 0x40; // real = 0x4000 = 16384 → 1.0

    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 18, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(pkt, 18);

    bno08x_rotation_vector_t rv = {0};
    TEST_ASSERT_EQUAL(BNO08X_OK, bno08x_read_rotation_vector(&rv));
    TEST_ASSERT_EQUAL_UINT8(2, rv.accuracy);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.25f, rv.i);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f,  rv.j);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f,  rv.k);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f,  rv.real);
}

void test_read_rotation_vector_skips_timebase_ref(void) {
    // pkt = 4 (SHTP hdr) + 5 (0xFB timebase ref) + 14 (0x05 rotation vector) = 23
    uint8_t hdr[4] = { 23, 0, 0, 0 };
    uint8_t pkt[23] = {0};
    pkt[2] = 3;     // CHAN_REPORTS
    pkt[4] = 0xFBu; // timebase ref (5 bytes, pkt[5..8] zeros)
    pkt[9]  = 0x05u; // rotation vector
    pkt[11] = 1;     // accuracy = 1
    pkt[13] = 0x00; pkt[14] = 0x10; // i = 0x1000 = 4096 → 0.25
    pkt[19] = 0x00; pkt[20] = 0x40; // real = 0x4000 = 16384 → 1.0

    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 23, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(pkt, 23);

    bno08x_rotation_vector_t rv = {0};
    TEST_ASSERT_EQUAL(BNO08X_OK, bno08x_read_rotation_vector(&rv));
    TEST_ASSERT_EQUAL_UINT8(1, rv.accuracy);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.25f, rv.i);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f,  rv.j);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f,  rv.k);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f,  rv.real);
}

void test_read_rotation_vector_err_on_wrong_channel(void) {
    uint8_t hdr[4] = { 18, 0, 0, 0 };
    uint8_t pkt[18] = {0};
    pkt[0] = 18; pkt[2] = 2; pkt[4] = 0x05u; // channel=2, not reports

    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 4, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(hdr, 4);
    i2c_read_ExpectAndReturn(BNO08X_ADDR, NULL, 18, I2C_OK);
    i2c_read_IgnoreArg_buf();
    i2c_read_ReturnArrayThruPtr_buf(pkt, 18);

    bno08x_rotation_vector_t rv = {0};
    TEST_ASSERT_EQUAL(BNO08X_ERR, bno08x_read_rotation_vector(&rv));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_ok_when_sensor_responds);
    RUN_TEST(test_init_err_when_i2c_fails);
    RUN_TEST(test_init_err_when_zero_length_header);
    RUN_TEST(test_init_err_when_drain_fails);
    RUN_TEST(test_init_ok_when_second_peek_nacks);
    RUN_TEST(test_get_product_id_parses_response);
    RUN_TEST(test_get_product_id_err_on_write_fail);
    RUN_TEST(test_get_product_id_err_on_read_fail);
    RUN_TEST(test_get_product_id_err_on_packet_read_fail);
    RUN_TEST(test_get_product_id_err_on_wrong_report_id);
    RUN_TEST(test_enable_rotation_vector_ok);
    RUN_TEST(test_enable_rotation_vector_err_on_write_fail);
    RUN_TEST(test_read_rotation_vector_parses_report);
    RUN_TEST(test_read_rotation_vector_skips_timebase_ref);
    RUN_TEST(test_read_rotation_vector_err_on_wrong_channel);
    return UNITY_END();
}
