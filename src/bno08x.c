#include "bno08x.h"
#include "i2c_bus.h"

#define SHTP_HDR_LEN        4u
#define SHTP_LEN_MASK       0x7FFFu
#define SHTP_ADVERT_MAX_LEN 512u  // advertisement is ~272 bytes; cap for sanity
#define CHAN_CONTROL         2u
#define CHAN_REPORTS         3u
#define REP_PRODUCT_ID_REQ  0xF9u
#define REP_PRODUCT_ID_RSP  0xF8u
#define PRODUCT_ID_RSP_LEN  16u
#define REP_SET_FEATURE_CMD 0xFDu
#define REP_ROTATION_VECTOR 0x05u
#define REP_TIMEBASE_REF    0xFBu
#define SET_FEATURE_LEN     17u
#define Q14                 (1.0f / 16384.0f)

static uint8_t s_seq;

static i2c_status_t shtp_write(uint8_t channel, const uint8_t *payload, uint8_t payload_len) {
    uint8_t buf[64];
    uint16_t total = (uint16_t)(SHTP_HDR_LEN + payload_len);
    buf[0] = (uint8_t)(total & 0xFFu);
    buf[1] = (uint8_t)(total >> 8);
    buf[2] = channel;
    buf[3] = s_seq++;
    for (uint8_t i = 0; i < payload_len; i++)
        buf[SHTP_HDR_LEN + i] = payload[i];
    return i2c_write(BNO08X_ADDR, buf, total);
}

bno08x_status_t bno08x_init(void) {
    // Drain all pending packets. After an MCU reset (without a BNO08X power
    // cycle) stale sensor reports remain queued; consuming them all leaves the
    // device ready for fresh requests.
    // pkt_len == 0 on the first peek means the device has nothing yet — caller
    // should retry. pkt_len == 0 after at least one packet means buffer empty.
    uint8_t buf[SHTP_ADVERT_MAX_LEN];
    int got_packet = 0;
    for (;;) {
        uint8_t hdr[SHTP_HDR_LEN];
        if (i2c_read(BNO08X_ADDR, hdr, SHTP_HDR_LEN) != I2C_OK)
            return got_packet ? BNO08X_OK : BNO08X_ERR;

        uint16_t pkt_len = (uint16_t)(hdr[0] | ((uint16_t)hdr[1] << 8)) & SHTP_LEN_MASK;
        if (pkt_len == 0)
            return got_packet ? BNO08X_OK : BNO08X_ERR;
        if (pkt_len > SHTP_ADVERT_MAX_LEN)
            return BNO08X_ERR;

        if (i2c_read(BNO08X_ADDR, buf, pkt_len) != I2C_OK)
            return BNO08X_ERR;
        got_packet = 1;
    }
}

bno08x_status_t bno08x_get_product_id(bno08x_product_id_t *out) {
    uint8_t req[] = { REP_PRODUCT_ID_REQ, 0x00u };
    if (shtp_write(CHAN_CONTROL, req, (uint8_t)sizeof(req)) != I2C_OK)
        return BNO08X_ERR;

    i2c_delay_ms(50);

    uint8_t hdr[SHTP_HDR_LEN];
    if (i2c_read(BNO08X_ADDR, hdr, SHTP_HDR_LEN) != I2C_OK)
        return BNO08X_ERR;

    uint16_t pkt_len = (uint16_t)(hdr[0] | ((uint16_t)hdr[1] << 8)) & SHTP_LEN_MASK;
    if (pkt_len < SHTP_HDR_LEN || pkt_len > SHTP_ADVERT_MAX_LEN)
        return BNO08X_ERR;

    uint8_t buf[SHTP_ADVERT_MAX_LEN];
    if (i2c_read(BNO08X_ADDR, buf, pkt_len) != I2C_OK)
        return BNO08X_ERR;

    if (buf[SHTP_HDR_LEN] != REP_PRODUCT_ID_RSP)
        return BNO08X_ERR;

    out->reset_cause = buf[SHTP_HDR_LEN + 1u];
    out->sw_major    = buf[SHTP_HDR_LEN + 2u];
    out->sw_minor    = buf[SHTP_HDR_LEN + 3u];
    out->sw_patch    = (uint16_t)(buf[SHTP_HDR_LEN + 12u]
                     | ((uint16_t)buf[SHTP_HDR_LEN + 13u] << 8));
    return BNO08X_OK;
}

bno08x_status_t bno08x_enable_rotation_vector(uint32_t interval_us) {
    uint8_t cmd[SET_FEATURE_LEN] = {0};
    cmd[0] = REP_SET_FEATURE_CMD;
    cmd[1] = REP_ROTATION_VECTOR;
    cmd[5] = (uint8_t)(interval_us);
    cmd[6] = (uint8_t)(interval_us >> 8);
    cmd[7] = (uint8_t)(interval_us >> 16);
    cmd[8] = (uint8_t)(interval_us >> 24);
    return shtp_write(CHAN_CONTROL, cmd, SET_FEATURE_LEN) == I2C_OK
        ? BNO08X_OK : BNO08X_ERR;
}

bno08x_status_t bno08x_read_rotation_vector(bno08x_rotation_vector_t *out) {
    uint8_t hdr[SHTP_HDR_LEN];
    if (i2c_read(BNO08X_ADDR, hdr, SHTP_HDR_LEN) != I2C_OK)
        return BNO08X_ERR;

    uint16_t pkt_len = (uint16_t)(hdr[0] | ((uint16_t)hdr[1] << 8)) & SHTP_LEN_MASK;
    if (pkt_len < SHTP_HDR_LEN || pkt_len > SHTP_ADVERT_MAX_LEN)
        return BNO08X_ERR;

    uint8_t buf[SHTP_ADVERT_MAX_LEN];
    if (i2c_read(BNO08X_ADDR, buf, pkt_len) != I2C_OK)
        return BNO08X_ERR;

    if (buf[2] != CHAN_REPORTS)
        return BNO08X_ERR;

    uint16_t off = SHTP_HDR_LEN;
    while (off < pkt_len) {
        uint8_t rid = buf[off];
        if (rid == REP_TIMEBASE_REF) { off += 5u; continue; }
        if (rid == REP_ROTATION_VECTOR && (uint16_t)(off + 12u) <= pkt_len) {
            out->accuracy = buf[off + 2u] & 0x03u;
            int16_t ri = (int16_t)((uint16_t)buf[off + 4u] | ((uint16_t)buf[off + 5u] << 8));
            int16_t rj = (int16_t)((uint16_t)buf[off + 6u] | ((uint16_t)buf[off + 7u] << 8));
            int16_t rk = (int16_t)((uint16_t)buf[off + 8u] | ((uint16_t)buf[off + 9u] << 8));
            int16_t rr = (int16_t)((uint16_t)buf[off + 10u] | ((uint16_t)buf[off + 11u] << 8));
            out->i    = (float)ri * Q14;
            out->j    = (float)rj * Q14;
            out->k    = (float)rk * Q14;
            out->real = (float)rr * Q14;
            return BNO08X_OK;
        }
        break;
    }
    return BNO08X_ERR;
}
