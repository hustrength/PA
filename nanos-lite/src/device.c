#include "common.h"
#include <amdev.h>

/* PA 3.3 */
#define KEYDOWN_MASK 0x8000


size_t serial_write(const void *buf, size_t offset, size_t len) {
    /* PA 3.3 */
    for (int i = 0; i < len; i++) {
        _putc(((char *) buf)[i]);
    }
    return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
        [_KEY_NONE] = "NONE",
        _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
    /* PA 3.3 */
    int key = read_key();
    bool down = false;
    if (key & KEYDOWN_MASK) {
        key ^= KEYDOWN_MASK;
        down = true;
    }
    if (key != _KEY_NONE) {
        sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
    } else {
        int time = uptime();
        sprintf(buf, "t %d\n", time);
    }
    return strlen(buf);
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    /* PA 3.3 */
    if (len + offset > strlen(dispinfo))len = strlen(dispinfo) - offset;
    strncpy(buf, &dispinfo[offset], len);
    return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    /* PA 3.3 */
    offset /= 4;
    int x = offset % screen_width();
    int y = offset / screen_width();
    draw_rect((uint32_t *) buf, x, y, len / 4, 1);
    return len;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
    /* PA 3.3 */
    draw_sync();
    return len;
}

void init_device() {
    Log("Initializing devices...");
    _ioe_init();

    // TODO: print the string to array `dispinfo` with the format
    // described in the Navy-apps convention
    /* PA 3.3 */
    sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
