#include <device/usbd_pvt.h>
#include <pico/binary_info/code.h>
#include <pico/stdio/driver.h>
#include <tusb.h>

#include "stdio_tinyusb_cdc.h"

static uint8_t stdio_tinyusb_cdc_itf;

static mutex_t stdio_tinyusb_cdc_mutex;
#ifndef NDEBUG
static uint8_t stdio_usb_core_num;
#endif

#if PICO_STDIO_TINYUSB_CDC_SUPPORT_CHARS_AVAILABLE_CALLBACK

static void (*chars_available_callback)(void *);

static void *chars_available_param;

#endif

static void stdio_tinyusb_cdc_out_chars(const char *buf, int length) {
    static uint64_t last_avail_time;
    if (!mutex_try_enter_block_until(&stdio_tinyusb_cdc_mutex, make_timeout_time_ms(PICO_STDIO_DEADLOCK_TIMEOUT_MS))) {
        return;
    }
    if (stdio_tinyusb_cdc_connected()) {
        for (int i = 0; i < length;) {
            int n = length - i;
            int avail = (int) tud_cdc_n_write_available(stdio_tinyusb_cdc_itf);
            if (n > avail) n = avail;
            if (n) {
                int n2 = (int) tud_cdc_n_write(stdio_tinyusb_cdc_itf, buf + i, (uint32_t) n);
                tud_task();
                tud_cdc_n_write_flush(stdio_tinyusb_cdc_itf);
                i += n2;
                last_avail_time = time_us_64();
            } else {
                tud_task();
                tud_cdc_n_write_flush(stdio_tinyusb_cdc_itf);
                if (
                    !stdio_tinyusb_cdc_connected()
                    || (
                        !tud_cdc_n_write_available(stdio_tinyusb_cdc_itf)
                        && time_us_64() > last_avail_time + PICO_STDIO_TINYUSB_CDC_STDOUT_TIMEOUT_US
                    )
                    ) {
                    break;
                }
            }
        }
    } else {
        // reset our timeout
        last_avail_time = 0;
    }
    mutex_exit(&stdio_tinyusb_cdc_mutex);
}

int stdio_tinyusb_cdc_in_chars(char *buf, int length) {
    // note we perform this check outside the lock, to try and prevent possible deadlock conditions
    // with printf in IRQs (which we will escape through timeouts elsewhere, but that would be less graceful).
    //
    // these are just checks of state, so we can call them while not holding the lock.
    // they may be wrong, but only if we are in the middle of a tud_task call, in which case at worst
    // we will mistakenly think we have data available when we do not (we will check again), or
    // tud_task will complete running and we will check the right values the next time.
    //
    int rc = PICO_ERROR_NO_DATA;
    if (stdio_tinyusb_cdc_connected() && tud_cdc_n_available(stdio_tinyusb_cdc_itf)) {
        if (!mutex_try_enter_block_until(&stdio_tinyusb_cdc_mutex,
                                         make_timeout_time_ms(PICO_STDIO_DEADLOCK_TIMEOUT_MS))) {
            return PICO_ERROR_NO_DATA; // would deadlock otherwise
        }
        if (stdio_tinyusb_cdc_connected() && tud_cdc_n_available(stdio_tinyusb_cdc_itf)) {
            int count = (int) tud_cdc_n_read(stdio_tinyusb_cdc_itf, buf, (uint32_t) length);
            rc = count ? count : PICO_ERROR_NO_DATA;
        } else {
            // because our mutex use may starve out the background task, run tud_task here (we own the mutex)
            tud_task();
        }
        mutex_exit(&stdio_tinyusb_cdc_mutex);
    }
    return rc;
}

#if PICO_STDIO_TINYUSB_CDC_SUPPORT_CHARS_AVAILABLE_CALLBACK

void tud_cdc_rx_cb(uint8_t itf) {
    if (itf == stdio_tinyusb_cdc_itf && chars_available_callback) {
        usbd_defer_func(chars_available_callback, chars_available_param, false);
    }
}

void stdio_tinyusb_cdc_set_chars_available_callback(void (*fn)(void *), void *param) {
    chars_available_callback = fn;
    chars_available_param = param;
}

#endif

stdio_driver_t stdio_tinyusb_cdc = {
    .out_chars = stdio_tinyusb_cdc_out_chars,
    .in_chars = stdio_tinyusb_cdc_in_chars,
#if PICO_STDIO_TINYUSB_CDC_SUPPORT_CHARS_AVAILABLE_CALLBACK
    .set_chars_available_callback = stdio_tinyusb_cdc_set_chars_available_callback,
#endif
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_TINYUSB_DEFAULT_CRLF
#endif
};

bool stdio_tinyusb_cdc_init(void) {
    return stdio_tinyusb_cdc_init_full(
        PICO_STDIO_TINYUSB_CDC_DEFAULT_ITF,
        PICO_STDIO_TINYUSB_CDC_CONNECT_WAIT_TIMEOUT_MS
    );
}

bool stdio_tinyusb_cdc_init_full(uint8_t itf, int32_t connect_timeout_ms) {
    if (get_core_num() != alarm_pool_core_num(alarm_pool_get_default())) {
        // included an assertion here rather than just returning false, as this is likely
        // a coding bug, rather than anything else.
        assert(false);
        return false;
    }
#ifndef NDEBUG
    stdio_usb_core_num = (uint8_t) get_core_num();
#endif
#if !PICO_NO_BI_STDIO_USB
    bi_decl_if_func_used(bi_program_feature("USB stdin / stdout"));
#endif

    assert(itf < CFG_TUD_CDC);
    // we expect the caller to have initialized if they are using TinyUSB
    assert(tud_inited());

    stdio_tinyusb_cdc_itf = itf;

    mutex_init(&stdio_tinyusb_cdc_mutex);
    stdio_set_driver_enabled(&stdio_tinyusb_cdc, true);
    if (connect_timeout_ms != 0) {
        absolute_time_t until = connect_timeout_ms > 0
                                ? make_timeout_time_ms(connect_timeout_ms)
                                : at_the_end_of_time;

        do {
            if (stdio_tinyusb_cdc_connected()) {
#if PICO_STDIO_USB_POST_CONNECT_WAIT_DELAY_MS != 0
                sleep_ms(PICO_STDIO_TINYUSB_CDC_POST_CONNECT_WAIT_DELAY_MS);
#endif
                break;
            }
            sleep_ms(10);
        } while (!time_reached(until));
    }

    return true;
}

bool stdio_tinyusb_cdc_connected() {
#if PICO_STDIO_TINYUSB_CDC_CONNECTION_WITHOUT_DTR
    return tud_ready();
#else
    // this actually checks DTR
    return tud_cdc_n_connected(stdio_tinyusb_cdc_itf);
#endif
}
