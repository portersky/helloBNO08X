#include "bno08x.h"
#include "i2c_bus.h"

#define SHTP_HDR_LEN        4u
#define SHTP_LEN_MASK       0x7FFFu
#define SHTP_ADVERT_MAX_LEN 512u  // advertisement is ~272 bytes; cap for sanity
#define CHAN_CONTROL         2u
#define REP_PRODUCT_ID_REQ  0xF9u
#define REP_PRODUCT_ID_RSP  0xF8u
#define PRODUCT_ID_RSP_LEN  16u

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
    // Peek at the 4-byte header to learn the total packet length.
    // A partial read does NOT advance the BNO08X pointer; only reading
    // exactly pkt_len bytes consumes the packet.
    uint8_t hdr[SHTP_HDR_LEN];
    if (i2c_read(BNO08X_ADDR, hdr, SHTP_HDR_LEN) != I2C_OK)
        return BNO08X_ERR;

    uint16_t pkt_len = (uint16_t)(hdr[0] | ((uint16_t)hdr[1] << 8)) & SHTP_LEN_MASK;
    if (pkt_len == 0 || pkt_len > SHTP_ADVERT_MAX_LEN)
        return BNO08X_ERR;

    // Consume the full advertisement so the buffer is clear before any requests.
    uint8_t buf[SHTP_ADVERT_MAX_LEN];
    return i2c_read(BNO08X_ADDR, buf, pkt_len) == I2C_OK ? BNO08X_OK : BNO08X_ERR;
}

bno08x_status_t bno08x_get_product_id(bno08x_product_id_t *out) {
    uint8_t req[] = { REP_PRODUCT_ID_REQ, 0x00u };
    if (shtp_write(CHAN_CONTROL, req, (uint8_t)sizeof(req)) != I2C_OK)
        return BNO08X_ERR;

    i2c_delay_ms(50);

    uint8_t rsp[SHTP_HDR_LEN + PRODUCT_ID_RSP_LEN];
    if (i2c_read(BNO08X_ADDR, rsp, (uint16_t)sizeof(rsp)) != I2C_OK)
        return BNO08X_ERR;

    if (rsp[SHTP_HDR_LEN] != REP_PRODUCT_ID_RSP)
        return BNO08X_ERR;

    out->reset_cause = rsp[SHTP_HDR_LEN + 1u];
    out->sw_major    = rsp[SHTP_HDR_LEN + 2u];
    out->sw_minor    = rsp[SHTP_HDR_LEN + 3u];
    out->sw_patch    = (uint16_t)(rsp[SHTP_HDR_LEN + 12u]
                     | ((uint16_t)rsp[SHTP_HDR_LEN + 13u] << 8));
    return BNO08X_OK;
}
